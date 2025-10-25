Project: DomPiWeb — guidance for AI coding agents

Keep this short and actionable. Focus on what a coding agent needs to be productive in this repo.

1) Big-picture architecture
- This repository contains multiple related projects: DomPiWeb (Raspberry Pi home hub), DomPiCloud (cloud services), and an embedded GMonitor transactional monitor in `gmonitor/system`.
- Runtime split: small C/C++ servers (under `Programas/Servers/*`) run as gmonitor servers; CGI programs live under `Programas/Clientes/*` and static site under `Programas/Html/*`. The gmonitor core (router/queue/listener/clients) is in `gmonitor/system` and is built/installed as a system service.
- IPC / messaging: services communicate via GMonitor messages (subscribe/notify/call). Look for calls to `Suscribe`, `UnSuscribe`, `Call`, `Notify` and constants like `GM_MSG_TYPE_CR`, `GM_MSG_TYPE_NOT`, `GM_MSG_TYPE_MSG`, `GM_MSG_TYPE_INT`. Example: `Programas/Servers/dompi_server/dompi_server.cc` subscribes to many events (e.g. `dompi_infoio`, `dompi_ass_on`).

2) Where to build and how
- Top-level build: run `make` in the project root (or in `DomPiWeb` and `gmonitor/system`) — Makefiles include `configure.mk` which centralizes install/build variables.
- Typical flow to build DomPiWeb locally:
  - cd DomPiWeb
  - make            # builds Programas (Clients + Servers)
  - make install    # runs Database, Script, Programas install targets (install may require root)
- To build the gmonitor core: cd `gmonitor/system` and run `make` / `make install`.
- Binaries are generated under `.tmp_$(hostname)/exe` (`PROG`) and object files under `.tmp_$(hostname)/obj`. Many server Makefiles set EXE and call an `install_server.sh` helper to install into `$(INST_SBINDIR)`.

3) Runtime & install notes (important for debugging)
- Config files are expected in `/etc` at runtime. Key examples:
  - `/etc/dompiweb.config` (source: `Config/dompiweb.config`) — contains DB connection and timeouts.
  - `/etc/dompicloud.config` for the cloud project.
- The installer script `Script/install.sh` documents required system packages: xinetd, apache2, php, libcjson, mysql, curl, openssl, etc. It also creates the `gmonitor` user, copies CGI and HTML files and initializes the database with `Database/create.sql` and `Database/init.sql`.
- Systemd: `install.sh` enables `gmonitor.service` and creates a `/etc/init.d/gmond` symlink to the installed `gmond` binary.

4) Project conventions & patterns
- All Makefiles include `configure.mk` for consistent paths (RUN_HOME, INST_*). Agents should modify `configure.mk` only when necessary and prefer to read variables from it.
- C/C++ conventions: code mixes C and C++ idioms. Common utilities and config loader live in `Programas/Common` (see `config.cc/h`). Use `DPConfig` to read runtime params.
- Message handling: servers typically create a `CGMServerWait`/`CGMServer` instance, call `Init("service_name")` and then `Suscribe(...)`. Use message types consistently: CR = request/response, NOT = notify, MSG = broadcast/event. See `gmonitor/system` implementation for reference (`queue/gmq.cc`, `router/gmt.cc`).
- JSON: code uses `cJSON` (`-lcjson`) for JSON payloads — ensure linker flags in Makefiles include `-lcjson` if touching build files.

5) Integration & cross-component communication
- Database: MySQL is used in DomPiWeb (Makefiles call `mysql_config` and `make install` runs SQL scripts). DB credentials come from config file keys `DBHOST`, `DBNAME`, `DBUSER`, `DBPASSWORD`.
- CGIs call into gmonitor via local IP/Unix socket patterns — examine `Programas/Clientes/*/*.cc` for `DPConfig` and client usage.
- Cloud integration: DomPiCloud contains CGI and servers that read `dompicloud.config` and interact with remote services (AWS, notifications). See `DomPiCloud/Programas/Servers/*` for examples.

6) Typical small tasks examples (explicit codebase examples)
- To add a new gmonitor server:
  - Create `Programas/Servers/<your_server>` with a `Makefile` mirroring `dompi_server/Makefile`, set EXE name and OBJECTS, include `../../../configure.mk`.
  - In code call `m_pServer = new CGMServerWait; m_pServer->Init("<service_name>"); m_pServer->Suscribe("<event>", GM_MSG_TYPE_CR);` and implement message processing in the main loop.
- To add a new CGI endpoint:
  - Add code under `Programas/Clientes/<name>.cgi/`, use `DPConfig("/etc/dompiweb.config")` to read settings, compile via existing `Clientes/Makefile` pattern and install to `$(RUN_HOME)/cgi`.

7) Where to look for authoritative examples
- Long server: `Programas/Servers/dompi_server/dompi_server.cc` (message subscriptions, DB usage).
- Client library patterns: `gmonitor/system/client/gmc.cc, gmc.h` (how to Call/Notify/Suscribe).
- Makefile conventions: `DomPiWeb/Makefile`, `DomPiWeb/configure.mk`, `gmonitor/system/Makefile` and per-server Makefiles (e.g., `dompi_server/Makefile`).

8) Short checklist for agents making edits
- Read `configure.mk` first to preserve build/install variables.
- When changing IPC event names, update all subscribers in both DomPiWeb and DomPiCloud (grep for the event string).
- When adding libraries, add flags to the corresponding Makefile and ensure `install` target copies any runtime assets.

If anything here is unclear or you'd like the doc to include examples for a specific change (add server, change a CGI, cross-build for ARM), tell me which task to expand and I will iterate.
