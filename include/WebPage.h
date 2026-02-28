#pragma once

namespace WebPage {

constexpr const char SETUP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Pengaturan Smart Server</title>
  <style>
    *{box-sizing:border-box;margin:0;padding:0}
    body{
      font-family:system-ui,-apple-system,sans-serif;background:#06080f;color:#eef0f6;
      padding:20px;min-height:100vh;
      background-image:
        radial-gradient(circle at 20% 20%, rgba(79,143,255,0.12), transparent 50%),
        radial-gradient(circle at 80% 80%, rgba(34,211,238,0.08), transparent 50%);
      background-attachment:fixed;
    }
    .app{max-width:860px;margin:0 auto;display:grid;gap:16px}
    .card{
      background:rgba(15,20,35,0.7);backdrop-filter:blur(20px);-webkit-backdrop-filter:blur(20px);
      border:1px solid rgba(255,255,255,0.06);border-radius:14px;padding:22px;
      box-shadow:0 8px 32px -8px rgba(0,0,0,0.4);
    }
    h1{font-size:1.4rem;font-weight:800;letter-spacing:-0.02em;margin-bottom:14px}
    h1 span{background:linear-gradient(135deg,#4f8fff,#22d3ee);-webkit-background-clip:text;-webkit-text-fill-color:transparent}
    h2{font-size:0.95rem;font-weight:700;margin-bottom:14px;color:#fff}
    .grid{display:grid;gap:12px;grid-template-columns:repeat(auto-fit,minmax(180px,1fr))}
    label{font-size:0.75rem;color:#7c86a2;display:block;margin-bottom:5px;font-weight:600;text-transform:uppercase;letter-spacing:0.04em}
    input,select{
      width:100%;padding:10px 12px;border-radius:8px;
      border:1px solid rgba(255,255,255,0.08);background:rgba(0,0,0,0.25);
      color:#eef0f6;font-size:0.88rem;outline:none;transition:border-color 0.2s,box-shadow 0.2s;
    }
    input:focus,select:focus{border-color:#4f8fff;box-shadow:0 0 0 2px rgba(79,143,255,0.15)}
    .row{display:flex;gap:10px;flex-wrap:wrap;margin-top:10px}
    button{
      padding:10px 18px;border-radius:8px;border:none;
      background:linear-gradient(135deg,#4f8fff,#3570e0);color:#fff;
      cursor:pointer;font-weight:600;font-size:0.82rem;
      transition:transform 0.2s,box-shadow 0.2s;
      box-shadow:0 4px 12px rgba(79,143,255,0.25);
    }
    button:hover{transform:translateY(-1px);box-shadow:0 6px 16px rgba(79,143,255,0.35)}
    button:active{transform:translateY(0)}
    button.danger{background:linear-gradient(135deg,#f87171,#dc2626);box-shadow:0 4px 12px rgba(248,113,113,0.25)}
    button.danger:hover{box-shadow:0 6px 16px rgba(248,113,113,0.35)}
    table{width:100%;border-collapse:separate;border-spacing:0;margin-top:10px}
    th,td{border-bottom:1px solid rgba(255,255,255,0.04);padding:10px 8px;text-align:left;font-size:0.82rem}
    th{color:#7c86a2;font-size:0.7rem;text-transform:uppercase;letter-spacing:0.05em;font-weight:600}
    .status{font-size:0.8rem;color:#34d399;margin-top:10px;min-height:18px;font-weight:500}
    .wifi-item{
      padding:10px 14px;border:1px solid rgba(255,255,255,0.06);border-radius:8px;
      cursor:pointer;margin-bottom:6px;background:rgba(0,0,0,0.2);transition:all 0.2s;
      font-size:0.88rem;
    }
    .wifi-item:hover{background:rgba(255,255,255,0.04);border-color:rgba(255,255,255,0.1)}
    .wifi-item.active{border-color:#4f8fff;background:rgba(79,143,255,0.08)}
  </style>
</head>
<body>
  <div class="app">
    <div class="card">
      <h1>Smart Server <span>Pengaturan</span></h1>
      <div class="row" style="margin-top:0"><a href="/" style="text-decoration:none"><button>&larr; Dashboard</button></a></div>
    </div>

    <div class="card">
      <h2>WiFi</h2>
      <div id="wifi-list"></div>
      <div class="row">
        <button onclick="scanWifi()">Pindai</button>
        <input id="wifi-pass" type="password" placeholder="Kata sandi WiFi" style="max-width:240px">
        <button onclick="connectWifi()">Hubungkan</button>
      </div>
      <div class="status" id="wifi-status"></div>
    </div>

    <div class="card">
      <h2>Konfigurasi Termal</h2>
      <div class="grid">
        <div><label>Ambang Peringatan (C)</label><input id="warn-th" type="number" step="0.1"></div>
        <div><label>Ambang Alarm (C)</label><input id="stage2-th" type="number" step="0.1"></div>
        <div><label>Kipas 1 Dasar</label><select id="fan1-baseline"><option value="true">NYALA</option><option value="false">MATI</option></select></div>
        <div><label>Interval Sensor (d)</label><input id="sensor-int" type="number" min="1"></div>
        <div><label>Interval Cloud (d)</label><input id="cloud-int" type="number" min="10"></div>
      </div>
      <div class="row"><button onclick="saveThermal()">Simpan Termal</button></div>
      <div class="status" id="thermal-status"></div>
    </div>

    <div class="card">
      <h2>Konfigurasi Keamanan</h2>
      <div class="grid">
        <div><label>Maks Percobaan Gagal</label><input id="max-fail" type="number" min="1"></div>
        <div><label>Penguncian (d)</label><input id="lockout-sec" type="number" min="10"></div>
        <div><label>Buka Solenoid (d)</label><input id="unlock-sec" type="number" min="1"></div>
        <div><label>ID Perangkat</label><input id="device-id" type="text"></div>
      </div>
      <div class="row"><button onclick="saveSecurity()">Simpan Keamanan</button></div>
      <div class="status" id="security-status"></div>
    </div>

    <div class="card">
      <h2>Manajemen PIN Pengguna</h2>
      <div class="grid">
        <div><label>ID Pengguna</label><input id="u-id" type="text" placeholder="admin1"></div>
        <div><label>Nama Tampilan</label><input id="u-name" type="text" placeholder="Admin 1"></div>
        <div><label>PIN (4-8 digit)</label><input id="u-pin" type="password" placeholder="1234"></div>
      </div>
      <div class="row">
        <button onclick="saveUser()">Tambah / Ubah</button>
        <button onclick="loadUsers()">Muat Ulang</button>
      </div>
      <table>
        <thead><tr><th>ID Pengguna</th><th>Nama</th><th>Aktif</th><th>Aksi</th></tr></thead>
        <tbody id="users-body"></tbody>
      </table>
      <div class="status" id="users-status"></div>
    </div>
  </div>

  <script>
    let selectedSsid = "";
    async function fetchJson(url, options) {
      const res = await fetch(url, options || {});
      const txt = await res.text();
      try { return JSON.parse(txt); } catch { return { raw: txt, ok: res.ok }; }
    }
    function setStatus(id, msg, isError=false) {
      const el = document.getElementById(id);
      el.style.color = isError ? "#f87171" : "#34d399";
      el.textContent = msg;
    }
    async function scanWifi() {
      setStatus("wifi-status", "Memindai...");
      try {
        const data = await fetchJson("/api/wifi/scan");
        const list = document.getElementById("wifi-list");
        list.innerHTML = "";
        (data.networks || []).forEach(n => {
          const div = document.createElement("div");
          div.className = "wifi-item";
          div.textContent = `${n.ssid} (${n.rssi} dBm)`;
          div.onclick = () => {
            selectedSsid = n.ssid;
            document.querySelectorAll(".wifi-item").forEach(x => x.classList.remove("active"));
            div.classList.add("active");
          };
          list.appendChild(div);
        });
        setStatus("wifi-status", "Pindai selesai");
      } catch (e) {
        setStatus("wifi-status", "Pindai gagal", true);
      }
    }
    async function connectWifi() {
      if (!selectedSsid) return setStatus("wifi-status", "Pilih SSID terlebih dahulu", true);
      const pass = document.getElementById("wifi-pass").value;
      const data = await fetchJson("/api/wifi/connect", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ ssid: selectedSsid, password: pass })
      });
      if (data.success) setStatus("wifi-status", "Terhubung: " + data.ip);
      else setStatus("wifi-status", data.error || "Gagal terhubung", true);
    }
    async function loadThermal() {
      const c = await fetchJson("/api/config/thermal");
      document.getElementById("warn-th").value = c.warnThreshold ?? 27;
      document.getElementById("stage2-th").value = c.stage2Threshold ?? 28;
      document.getElementById("fan1-baseline").value = String(c.fan1BaselineOn ?? true);
      document.getElementById("sensor-int").value = c.sensorReadIntervalSec ?? 5;
      document.getElementById("cloud-int").value = c.cloudSendIntervalSec ?? 60;
    }
    async function saveThermal() {
      const payload = {
        warnThreshold: parseFloat(document.getElementById("warn-th").value),
        stage2Threshold: parseFloat(document.getElementById("stage2-th").value),
        fan1BaselineOn: document.getElementById("fan1-baseline").value === "true",
        sensorReadIntervalSec: parseInt(document.getElementById("sensor-int").value),
        cloudSendIntervalSec: parseInt(document.getElementById("cloud-int").value)
      };
      const res = await fetch("/api/config/thermal", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
      });
      setStatus("thermal-status", res.ok ? "Tersimpan" : "Gagal menyimpan", !res.ok);
    }
    async function loadSecurity() {
      const c = await fetchJson("/api/config/security");
      document.getElementById("max-fail").value = c.maxFail ?? 3;
      document.getElementById("lockout-sec").value = c.lockoutSecs ?? 120;
      document.getElementById("unlock-sec").value = c.unlockSecs ?? 10;
      document.getElementById("device-id").value = c.deviceId ?? "";
    }
    async function saveSecurity() {
      const payload = {
        maxFail: parseInt(document.getElementById("max-fail").value),
        lockoutSecs: parseInt(document.getElementById("lockout-sec").value),
        unlockSecs: parseInt(document.getElementById("unlock-sec").value),
        deviceId: document.getElementById("device-id").value
      };
      const res = await fetch("/api/config/security", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
      });
      setStatus("security-status", res.ok ? "Tersimpan" : "Gagal menyimpan", !res.ok);
    }
    async function loadUsers() {
      const data = await fetchJson("/api/users");
      const body = document.getElementById("users-body");
      body.innerHTML = "";
      (data.users || []).forEach(u => {
        const tr = document.createElement("tr");
        tr.innerHTML = `<td>${u.userId}</td><td>${u.displayName || ""}</td><td>${u.enabled ? "Ya" : "Tidak"}</td>
          <td><button class="danger" onclick="deleteUser('${u.userId}')">Hapus</button></td>`;
        body.appendChild(tr);
      });
      setStatus("users-status", `${data.count || 0} pengguna dimuat`);
    }
    async function saveUser() {
      const payload = {
        userId: document.getElementById("u-id").value.trim(),
        displayName: document.getElementById("u-name").value.trim(),
        pin: document.getElementById("u-pin").value.trim(),
        enabled: true
      };
      const res = await fetch("/api/users", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
      });
      if (res.ok) {
        setStatus("users-status", "Pengguna tersimpan");
        document.getElementById("u-pin").value = "";
        loadUsers();
      } else {
        const err = await res.text();
        setStatus("users-status", "Gagal: " + err, true);
      }
    }
    async function deleteUser(userId) {
      const res = await fetch(`/api/users/${encodeURIComponent(userId)}`, { method: "DELETE" });
      if (res.ok) {
        setStatus("users-status", "Pengguna dihapus");
        loadUsers();
      } else {
        setStatus("users-status", "Gagal menghapus", true);
      }
    }
    scanWifi();
    loadThermal();
    loadSecurity();
    loadUsers();
  </script>
</body>
</html>
)rawliteral";

constexpr const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Dashboard Smart Server</title>
  <style>
    *{box-sizing:border-box;margin:0;padding:0}
    body{
      font-family:system-ui,-apple-system,sans-serif;background:#06080f;color:#eef0f6;
      padding:20px;min-height:100vh;
      background-image:radial-gradient(circle at top right,rgba(79,143,255,0.1),transparent 50%);
      background-attachment:fixed;
    }
    .app{max-width:800px;margin:0 auto;display:grid;gap:16px}
    .card{
      background:rgba(15,20,35,0.7);backdrop-filter:blur(20px);-webkit-backdrop-filter:blur(20px);
      border:1px solid rgba(255,255,255,0.06);border-radius:14px;padding:22px;
      box-shadow:0 8px 32px -8px rgba(0,0,0,0.4);
    }
    h1{font-size:1.5rem;font-weight:800;letter-spacing:-0.02em;margin-bottom:16px}
    h1 span{background:linear-gradient(135deg,#4f8fff,#22d3ee);-webkit-background-clip:text;-webkit-text-fill-color:transparent}
    .metrics{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-bottom:16px}
    .metric{background:rgba(0,0,0,0.2);padding:16px;border-radius:10px;border:1px solid rgba(255,255,255,0.04)}
    .metric-label{font-size:0.72rem;color:#7c86a2;text-transform:uppercase;letter-spacing:0.05em;font-weight:600;margin-bottom:6px}
    .metric-val{font-size:2.2rem;font-weight:800;color:#fff;line-height:1}
    .info{
      font-family:ui-monospace,SFMono-Regular,Consolas,monospace;font-size:0.82rem;
      color:#93afd4;background:rgba(0,0,0,0.25);padding:10px 12px;border-radius:8px;
      margin-top:8px;line-height:1.6;
    }
    .actions{display:flex;gap:10px;margin-top:12px;flex-wrap:wrap}
    button,a.btn{
      padding:10px 18px;border-radius:8px;border:none;
      background:linear-gradient(135deg,#4f8fff,#3570e0);color:#fff;
      text-decoration:none;display:inline-flex;align-items:center;
      font-weight:600;font-size:0.82rem;cursor:pointer;
      box-shadow:0 4px 12px rgba(79,143,255,0.25);transition:all 0.2s;
    }
    button:hover,a.btn:hover{transform:translateY(-1px);box-shadow:0 6px 16px rgba(79,143,255,0.35)}
    button:active{transform:translateY(0)}
  </style>
</head>
<body>
  <div class="app">
    <div class="card">
      <h1>Smart Server <span>Dashboard</span></h1>
      <div class="metrics">
        <div class="metric">
          <div class="metric-label">Suhu</div>
          <div class="metric-val" id="temp">--</div>
        </div>
        <div class="metric">
          <div class="metric-label">Kelembapan</div>
          <div class="metric-val" id="hum">--</div>
        </div>
      </div>
      <div class="info" id="status">Memuat...</div>
      <div class="info" id="security">Keamanan: --</div>
      <div class="actions">
        <button onclick="sendNow()">Kirim Sekarang</button>
        <a class="btn" href="/setup">Pengaturan</a>
      </div>
    </div>
  </div>
  <script>
    async function refresh() {
      try {
        const res = await fetch('/api/state');
        const d = await res.json();
        document.getElementById('temp').textContent = d.valid ? d.temperature.toFixed(1) + ' °C' : '--';
        document.getElementById('hum').textContent = d.valid ? d.humidity.toFixed(1) + ' %' : '--';
        document.getElementById('status').textContent =
          `WiFi: ${d.wifiConnected ? 'ON' : 'OFF'}  |  K1: ${d.fan1On ? 'ON' : 'OFF'}  K2: ${d.fan2On ? 'ON' : 'OFF'}  |  Alarm: ${d.alarm ? 'YA' : 'TIDAK'}  |  Antrean: ${d.queueTelemetry}/${d.queueAccess}`;
        document.getElementById('security').textContent =
          `Keamanan: ${d.accessMessage || '-'}  |  Terkunci: ${d.lockoutActive ? ('YA ' + d.lockoutRemainingSec + 'd') : 'TIDAK'}  |  Pintu: ${d.doorState}`;
      } catch (e) {
        document.getElementById('status').textContent = 'Kesalahan koneksi';
      }
    }
    async function sendNow() {
      await fetch('/api/send', { method: 'POST' });
      refresh();
    }
    refresh();
    setInterval(refresh, 3000);
  </script>
</body>
</html>
)rawliteral";

}  // namespace WebPage
