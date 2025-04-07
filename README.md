### 🧠 **Malware Name**: `NoHaX`

**Type**: Cryptojacking Daemon  
**Author**: Mohak  
**Target OS**: Linux (x86_64)  
**Mining Coin**: Monero (XMR)  
**Stealth Grade**: ★★★★★ (Daemonized, Background Execution, RAM-based)

---

### 📝 **Description**:

`NoHaX` is a covert, autonomous cryptocurrency mining daemon engineered for persistence and stealth. Designed to run silently in the background as a Unix daemon, it downloads and launches the latest stable build of XMRig from a trusted upstream source, targeting Monero pools with TLS encryption for obfuscation.

Once deployed, `NoHaX`:
- Detaches from the terminal using a double-fork `daemonize()` pattern
- Executes XMRig with preconfigured pool and wallet details
- Redirects output to `/tmp/xmrig.log` for low-profile monitoring
- Disables interaction by closing STDIN/STDOUT/STDERR
- Masks its presence by using `nohup`, forked execution, and optionally systemd timers

While still in development, future versions of `NoHaX` could:
- Auto-persist via `~/.config/systemd/user/` services
- Use obfuscated binaries with embedded miners
- Encrypt its configuration for extra stealth

---

### 🛑 **Disclaimer**:
This tool is meant for **educational purposes only**. Unauthorized use of malware, miners, or persistence techniques on systems you don’t own or manage is illegal and unethical.

---
