#pragma once

namespace WebPage {

constexpr const char SETUP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Server Setup</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: "Segoe UI", sans-serif;
      background: #0f172a;
      color: #e2e8f0;
      padding: 20px;
    }
    .container { max-width: 960px; margin: 0 auto; display: grid; gap: 16px; }
    .card {
      background: #1e293b;
      border: 1px solid #334155;
      border-radius: 12px;
      padding: 16px;
    }
    h1 { font-size: 1.4rem; margin-bottom: 12px; color: #22d3ee; }
    h2 { font-size: 1rem; margin-bottom: 12px; color: #94a3b8; }
    .grid { display: grid; gap: 12px; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); }
    label { font-size: .85rem; color: #cbd5e1; display: block; margin-bottom: 4px; }
    input, select {
      width: 100%;
      padding: 10px;
      border-radius: 8px;
      border: 1px solid #475569;
      background: #0f172a;
      color: #f8fafc;
    }
    button {
      padding: 10px 14px;
      border-radius: 8px;
      border: none;
      background: #0284c7;
      color: white;
      cursor: pointer;
      font-weight: 600;
    }
    button.danger { background: #b91c1c; }
    .row { display: flex; gap: 8px; flex-wrap: wrap; }
    table { width: 100%; border-collapse: collapse; }
    th, td { border-bottom: 1px solid #334155; padding: 8px; text-align: left; font-size: .9rem; }
    .status { font-size: .85rem; color: #22c55e; margin-top: 8px; min-height: 20px; }
    .wifi-item {
      padding: 8px 10px;
      border: 1px solid #334155;
      border-radius: 8px;
      cursor: pointer;
      margin-bottom: 6px;
      background: #0b1220;
    }
    .wifi-item.active { border-color: #22d3ee; }
  </style>
</head>
<body>
  <div class="container">
    <div class="card">
      <h1>Smart Server Setup</h1>
      <div class="row">
        <a href="/" style="text-decoration:none;"><button>Back to Dashboard</button></a>
      </div>
    </div>

    <div class="card">
      <h2>WiFi</h2>
      <div id="wifi-list"></div>
      <div class="row" style="margin-top:8px;">
        <button onclick="scanWifi()">Scan</button>
        <input id="wifi-pass" type="password" placeholder="WiFi password" style="max-width:280px;">
        <button onclick="connectWifi()">Connect</button>
      </div>
      <div class="status" id="wifi-status"></div>
    </div>

    <div class="card">
      <h2>Thermal Config</h2>
      <div class="grid">
        <div><label>Warn Threshold (C)</label><input id="warn-th" type="number" step="0.1"></div>
        <div><label>Stage2 Threshold (C)</label><input id="stage2-th" type="number" step="0.1"></div>
        <div><label>Fan1 Baseline</label><select id="fan1-baseline"><option value="true">ON</option><option value="false">OFF</option></select></div>
        <div><label>Sensor Interval (s)</label><input id="sensor-int" type="number" min="1"></div>
        <div><label>Cloud Interval (s)</label><input id="cloud-int" type="number" min="10"></div>
      </div>
      <div class="row" style="margin-top:10px;"><button onclick="saveThermal()">Save Thermal</button></div>
      <div class="status" id="thermal-status"></div>
    </div>

    <div class="card">
      <h2>Security Config</h2>
      <div class="grid">
        <div><label>Max Failed Attempts</label><input id="max-fail" type="number" min="1"></div>
        <div><label>Lockout (s)</label><input id="lockout-sec" type="number" min="10"></div>
        <div><label>Solenoid Unlock (s)</label><input id="unlock-sec" type="number" min="1"></div>
        <div><label>Device ID</label><input id="device-id" type="text"></div>
      </div>
      <div class="row" style="margin-top:10px;"><button onclick="saveSecurity()">Save Security</button></div>
      <div class="status" id="security-status"></div>
    </div>

    <div class="card">
      <h2>User PIN Management</h2>
      <div class="grid">
        <div><label>User ID</label><input id="u-id" type="text" placeholder="admin1"></div>
        <div><label>Display Name</label><input id="u-name" type="text" placeholder="Admin 1"></div>
        <div><label>PIN (4-8 digit)</label><input id="u-pin" type="password" placeholder="1234"></div>
      </div>
      <div class="row" style="margin-top:10px;">
        <button onclick="saveUser()">Add / Update User</button>
        <button onclick="loadUsers()">Refresh Users</button>
      </div>
      <table style="margin-top:10px;">
        <thead><tr><th>User ID</th><th>Name</th><th>Enabled</th><th>Action</th></tr></thead>
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
      el.style.color = isError ? "#ef4444" : "#22c55e";
      el.textContent = msg;
    }

    async function scanWifi() {
      setStatus("wifi-status", "Scanning...");
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
        setStatus("wifi-status", "Scan done");
      } catch (e) {
        setStatus("wifi-status", "Scan failed", true);
      }
    }

    async function connectWifi() {
      if (!selectedSsid) return setStatus("wifi-status", "Select SSID first", true);
      const pass = document.getElementById("wifi-pass").value;
      const data = await fetchJson("/api/wifi/connect", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ ssid: selectedSsid, password: pass })
      });
      if (data.success) setStatus("wifi-status", "Connected: " + data.ip);
      else setStatus("wifi-status", data.error || "Connect failed", true);
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
      setStatus("thermal-status", res.ok ? "Saved" : "Save failed", !res.ok);
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
      setStatus("security-status", res.ok ? "Saved" : "Save failed", !res.ok);
    }

    async function loadUsers() {
      const data = await fetchJson("/api/users");
      const body = document.getElementById("users-body");
      body.innerHTML = "";
      (data.users || []).forEach(u => {
        const tr = document.createElement("tr");
        tr.innerHTML = `<td>${u.userId}</td><td>${u.displayName || ""}</td><td>${u.enabled ? "Yes" : "No"}</td>
          <td><button class="danger" onclick="deleteUser('${u.userId}')">Delete</button></td>`;
        body.appendChild(tr);
      });
      setStatus("users-status", `Loaded ${data.count || 0} users`);
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
        setStatus("users-status", "User saved");
        document.getElementById("u-pin").value = "";
        loadUsers();
      } else {
        const err = await res.text();
        setStatus("users-status", "Save failed: " + err, true);
      }
    }

    async function deleteUser(userId) {
      const res = await fetch(`/api/users/${encodeURIComponent(userId)}`, { method: "DELETE" });
      if (res.ok) {
        setStatus("users-status", "User deleted");
        loadUsers();
      } else {
        setStatus("users-status", "Delete failed", true);
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
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Server Dashboard</title>
  <style>
    body { font-family: "Segoe UI", sans-serif; background:#020617; color:#e2e8f0; padding:20px; }
    .wrap { max-width: 760px; margin: 0 auto; display:grid; gap:16px; }
    .card { background:#0f172a; border:1px solid #1e293b; border-radius:12px; padding:16px; }
    h1 { color:#22d3ee; font-size:1.4rem; margin-bottom:12px; }
    .grid { display:grid; grid-template-columns:repeat(2,minmax(0,1fr)); gap:12px; }
    .v { font-size:1.8rem; font-weight:700; }
    button,a { padding:10px 12px; border-radius:8px; border:none; background:#0284c7; color:#fff; text-decoration:none; display:inline-block; }
    .mono { font-family: Consolas, monospace; font-size:.9rem; color:#93c5fd; }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="card">
      <h1>Smart Server Dashboard</h1>
      <div class="grid">
        <div><div>Temperature</div><div class="v" id="temp">--</div></div>
        <div><div>Humidity</div><div class="v" id="hum">--</div></div>
      </div>
      <div style="margin-top:10px;" class="mono" id="status">Loading...</div>
      <div style="margin-top:10px;" class="mono" id="security">Security: --</div>
      <div style="margin-top:10px;">
        <button onclick="sendNow()">Send Now</button>
        <a href="/setup">Settings</a>
      </div>
    </div>
  </div>
  <script>
    async function refresh() {
      try {
        const res = await fetch('/api/state');
        const d = await res.json();
        document.getElementById('temp').textContent = d.valid ? d.temperature.toFixed(1) + ' C' : '--';
        document.getElementById('hum').textContent = d.valid ? d.humidity.toFixed(1) + ' %' : '--';
        document.getElementById('status').textContent =
          `WiFi:${d.wifiConnected ? 'ON' : 'OFF'} | F1:${d.fan1On ? 'ON' : 'OFF'} F2:${d.fan2On ? 'ON' : 'OFF'} | Alarm:${d.alarm ? 'YES' : 'NO'} | Queue:${d.queueTelemetry}/${d.queueAccess}`;
        document.getElementById('security').textContent =
          `Security: ${d.accessMessage || '-'} | Lockout:${d.lockoutActive ? ('YES ' + d.lockoutRemainingSec + 's') : 'NO'} | Door:${d.doorState}`;
      } catch (e) {
        document.getElementById('status').textContent = 'Connection error';
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
