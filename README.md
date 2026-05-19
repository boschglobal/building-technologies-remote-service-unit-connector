# RSU Connector

RSU (Remote Service Unit) Connector repository contains the source code for the open source version of a Azure IoT Hub connector to a service unit. The service unit connector is based on a PnP schema (see `src/pnp-schema/RemoteServiceUnit.json`) which defines the capabilities.

## Usage

To build the RSU Connector for openWRT OS, please refer to the [How to build.md](./docs/How-to-build.md).

## Proxy authentication

The connector talks to Azure IoT Hub and the Device Provisioning Service through an optional HTTP CONNECT proxy. Behaviour is controlled by the `Proxy_*` keys in the connector config (deployed as `/etc/rsu-connector/rsu-connector-config.json`; template at [`openwrt/etc/config/rsu-connector-config.json`](./openwrt/etc/config/rsu-connector-config.json)). Leaving `Proxy_Host` empty or `Proxy_Port` at `0` disables the proxy entirely.

Three authentication modes are supported.

### Anonymous proxy (no auth)

Use when the proxy permits unauthenticated CONNECT to the Azure endpoints.

```json
{
    "Proxy_Host":       "proxy.example.com",
    "Proxy_Port":       3128,
    "Proxy_Username":   "",
    "Proxy_Password":   "",
    "Proxy_AuthMethod": "basic"
}
```

No `Proxy-Authorization` header is sent. (The startup log line `(auth=basic)` is a label, not a guarantee that credentials were transmitted — with empty user/password the connector sends nothing.)

### Basic auth

```json
{
    "Proxy_Host":       "proxy.example.com",
    "Proxy_Port":       3128,
    "Proxy_Username":   "alice",
    "Proxy_Password":   "s3cret",
    "Proxy_AuthMethod": "basic"
}
```

The connector sends `Proxy-Authorization: Basic base64(user:pass)` on every CONNECT. Because the password is stored in plain text in the config file, restrict the file's permissions on the device (`chmod 600`, owner = the user `rsu-connector` runs as, typically `root`).

### Kerberos / Negotiate (SPNEGO)

```json
{
    "Proxy_Host":       "proxy.corp.example.com",
    "Proxy_Port":       3128,
    "Proxy_Username":   "",
    "Proxy_Password":   "",
    "Proxy_AuthMethod": "negotiate",
    "Proxy_Keytab":     "/etc/rsu-connector/rsu.keytab"
}
```

`Proxy_AuthMethod` accepts `negotiate`, `kerberos`, or `spnego` (case-insensitive — all three select the same SPNEGO path). When `Proxy_Keytab` is set, the connector exports `KRB5_CLIENT_KTNAME` to that path and isolates the ticket cache at `KRB5CCNAME=FILE:/tmp/rsu-connector-krb5cc` so GSSAPI/libcurl auto-acquire a TGT from the keytab on first connect. If `Proxy_Keytab` is empty, the connector falls back to the ambient credential cache (whatever ticket is at `/tmp/krb5cc_<uid>` or wherever `KRB5CCNAME` already points) — meaning something else, e.g. an init script running `kinit`, has to put a valid ticket there first.

`Proxy_Username` / `Proxy_Password` may be left empty; if both are set they are kept as a Basic fallback for proxies that do not advertise `Negotiate`.

#### Device prerequisites for Kerberos

Required in addition to the config above. Missing any one of them causes the Negotiate path to fail before a token is ever sent, and the proxy rejects the CONNECT.

| Requirement | Detail |
|---|---|
| `/etc/krb5.conf` on the device | At minimum a `[libdefaults]` block with `default_realm`, plus either `dns_lookup_kdc = true` (when the KDC publishes `_kerberos._tcp` SRV records) or an explicit `[realms]` entry with `kdc = <host>`. The OpenWrt `krb5-libs` package does **not** install a default `krb5.conf`. |
| DNS that resolves the proxy's **FQDN** | Kerberos builds the service principal as `HTTP/<proxy-fqdn>@REALM`; an IP address in `Proxy_Host` will not authenticate, even if the IP routes correctly. |
| Clock sync within ~5 min of the KDC | Kerberos refuses skewed tickets. Run `ntpd` / `chronyd` on the device. |
| Keytab file readable by the connector user | Typically `chown root:root`, `chmod 600 /etc/rsu-connector/rsu.keytab`. The principal in the keytab is the device's client identity, not the proxy's SPN. |

## Contribute

Please refer to the [CONTRIBUTING.md](./CONTRIBUTING.md) for a quick read-up about what to consider if you want to contribute.

## Contributors

Contact details of the **authors** and **contributors** see [NOTICE](NOTICE).

## License

Building Technologies - RSU Connector is open-sourced under the `Apache-2.0` license. See the [LICENSE](./LICENSE) and [NOTICE](./NOTICE) file for details.