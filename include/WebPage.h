#pragma once

namespace WebPage {

constexpr const char SETUP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>Pengaturan — Smart Server</title>
<style>
:root{--bg:#060a13;--sf:rgba(12,17,32,.82);--bd:rgba(255,255,255,.07);--bh:rgba(255,255,255,.14);--tx:#edf0f7;--td:#6b7a99;--bl:#5b9aff;--cy:#22d3ee;--gn:#34d399;--rd:#f87171;--am:#fbbf24;--r:14px;--rs:10px}
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--tx);min-height:100vh;background-image:radial-gradient(ellipse at 15% 5%,rgba(91,154,255,.1),transparent 55%),radial-gradient(ellipse at 85% 95%,rgba(34,211,238,.07),transparent 55%);background-attachment:fixed}
.app{max-width:880px;margin:0 auto;padding:20px 16px 48px;display:flex;flex-direction:column;gap:16px}
.card{background:var(--sf);backdrop-filter:blur(24px);-webkit-backdrop-filter:blur(24px);border:1px solid var(--bd);border-radius:var(--r);padding:24px;transition:border-color .3s}
.hdr{display:flex;justify-content:space-between;align-items:center;padding:18px 24px}
.bt{font-size:.6rem;font-weight:700;text-transform:uppercase;letter-spacing:.2em;color:var(--bl);background:rgba(91,154,255,.1);padding:3px 10px;border-radius:6px;margin-bottom:4px;display:inline-block}
h1{font-size:1.35rem;font-weight:800;letter-spacing:-.02em;color:#fff}
.grd{background:linear-gradient(135deg,var(--bl),var(--cy));-webkit-background-clip:text;-webkit-text-fill-color:transparent}
.tabs{display:flex;gap:4px;background:rgba(0,0,0,.25);padding:4px;border-radius:var(--rs);overflow-x:auto}
.tb{flex:1;padding:10px 16px;border:none;background:0 0;color:var(--td);font-weight:600;font-size:.82rem;border-radius:8px;cursor:pointer;transition:all .25s;white-space:nowrap;font-family:inherit}
.tb.active{background:var(--bl);color:#fff;box-shadow:0 2px 12px rgba(91,154,255,.3)}
.tb:hover:not(.active){color:var(--tx);background:rgba(255,255,255,.04)}
.tp{display:none}.tp.active{display:block;animation:fadeIn .3s ease}
@keyframes fadeIn{from{opacity:0;transform:translateY(8px)}to{opacity:1;transform:translateY(0)}}
h2{font-size:.92rem;font-weight:700;color:#fff;margin-bottom:16px;display:flex;align-items:center;gap:8px}
h2 .tag{font-size:.62rem;font-weight:700;color:var(--td);background:rgba(255,255,255,.06);padding:2px 8px;border-radius:12px;text-transform:uppercase;letter-spacing:.04em}
.grid{display:grid;gap:12px;grid-template-columns:repeat(auto-fit,minmax(170px,1fr))}
label{font-size:.7rem;color:var(--td);display:block;margin-bottom:5px;font-weight:600;text-transform:uppercase;letter-spacing:.05em}
input,select{width:100%;padding:10px 14px;border-radius:var(--rs);border:1px solid var(--bd);background:rgba(0,0,0,.3);color:var(--tx);font-size:.85rem;outline:none;transition:border-color .2s,box-shadow .2s;font-family:inherit}
input:focus,select:focus{border-color:var(--bl);box-shadow:0 0 0 3px rgba(91,154,255,.12)}
select{cursor:pointer;appearance:none;background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='12' height='12' viewBox='0 0 24 24' fill='none' stroke='%236b7a99' stroke-width='2'%3E%3Cpath d='M6 9l6 6 6-6'/%3E%3C/svg%3E");background-repeat:no-repeat;background-position:right 12px center}
.acts{display:flex;gap:10px;margin-top:14px;flex-wrap:wrap}
.btn{padding:10px 20px;border-radius:var(--rs);border:none;font-weight:600;font-size:.82rem;cursor:pointer;transition:all .2s;display:inline-flex;align-items:center;gap:6px;font-family:inherit;text-decoration:none}
.bp{background:linear-gradient(135deg,var(--bl),#3d78e0);color:#fff;box-shadow:0 4px 14px rgba(91,154,255,.25)}
.bp:hover{transform:translateY(-1px);box-shadow:0 6px 20px rgba(91,154,255,.35)}
.bp:active{transform:translateY(0)}
.bs{background:rgba(255,255,255,.05);color:var(--tx);border:1px solid var(--bd)}
.bs:hover{background:rgba(255,255,255,.08);border-color:var(--bh)}
.bd{background:linear-gradient(135deg,var(--rd),#dc2626);color:#fff;box-shadow:0 4px 14px rgba(248,113,113,.2);padding:6px 14px;font-size:.75rem}
.bd:hover{box-shadow:0 6px 20px rgba(248,113,113,.3);transform:translateY(-1px)}
.wi{padding:12px 16px;border:1px solid var(--bd);border-radius:var(--rs);cursor:pointer;margin-bottom:8px;background:rgba(0,0,0,.2);transition:all .2s;display:flex;align-items:center;justify-content:space-between}
.wi:hover{background:rgba(255,255,255,.03);border-color:var(--bh)}
.wi.active{border-color:var(--bl);background:rgba(91,154,255,.06)}
.wn{font-size:.88rem;font-weight:500}
.wm{font-size:.72rem;color:var(--td);display:flex;align-items:center;gap:8px}
.sb{display:flex;gap:2px;align-items:flex-end;height:14px}
.sb span{width:3px;border-radius:1px}
table{width:100%;border-collapse:separate;border-spacing:0;margin-top:12px;border-radius:var(--rs);overflow:hidden;border:1px solid var(--bd)}
th,td{padding:11px 14px;text-align:left;font-size:.82rem}
th{background:rgba(0,0,0,.2);color:var(--td);font-size:.68rem;text-transform:uppercase;letter-spacing:.05em;font-weight:600;border-bottom:1px solid var(--bd)}
td{border-bottom:1px solid rgba(255,255,255,.03)}
tr:last-child td{border-bottom:none}
tbody tr{transition:background .15s}
tbody tr:hover{background:rgba(255,255,255,.03)}
.tbox{position:fixed;bottom:20px;right:20px;display:flex;flex-direction:column;gap:8px;z-index:999}
.tst{padding:12px 18px;border-radius:var(--rs);font-size:.82rem;font-weight:600;animation:sIn .35s ease;transition:opacity .3s,transform .3s;backdrop-filter:blur(16px);border:1px solid var(--bd);max-width:320px}
.tst.ok{background:rgba(52,211,153,.12);color:var(--gn);border-color:rgba(52,211,153,.2)}
.tst.er{background:rgba(248,113,113,.12);color:var(--rd);border-color:rgba(248,113,113,.2)}
@keyframes sIn{from{transform:translateX(100%);opacity:0}to{transform:translateX(0);opacity:1}}
@media(max-width:600px){.app{padding:14px 10px 32px}.card{padding:18px}.hdr{flex-direction:column;align-items:flex-start;gap:10px}.tabs{overflow-x:auto}.tb{padding:8px 12px;font-size:.78rem}}
</style>
</head>
<body>
<div class="app">
<header class="card hdr">
<div><div class="bt">Smart Server</div><h1><span class="grd">Pengaturan</span></h1></div>
<a href="/" style="text-decoration:none"><button class="btn bs">&larr; Dashboard</button></a>
</header>
<div class="tabs">
<button class="tb active" data-tab="wifi" onclick="sT('wifi')">WiFi</button>
<button class="tb" data-tab="thermal" onclick="sT('thermal')">Termal</button>
<button class="tb" data-tab="security" onclick="sT('security')">Keamanan</button>
<button class="tb" data-tab="users" onclick="sT('users')">Pengguna</button>
</div>
<div class="tp active" id="tab-wifi"><div class="card">
<h2>Jaringan WiFi <span class="tag">Pindai &amp; Hubungkan</span></h2>
<div id="wifi-list"></div>
<div class="grid" style="margin-top:12px">
<div><label>SSID</label><input id="wifi-ssid" type="text" placeholder="Masukkan SSID manual"></div>
<div><label>Kata Sandi</label><input id="wifi-pass" type="password" placeholder="Kata sandi WiFi"></div>
</div>
<div class="acts"><button class="btn bs" onclick="scanWifi()">Pindai Ulang</button><button class="btn bp" onclick="connectWifi()">Hubungkan</button></div>
</div></div>
<div class="tp" id="tab-thermal"><div class="card">
<h2>Konfigurasi Termal <span class="tag">Threshold &amp; Interval</span></h2>
<div class="grid">
<div><label>Ambang Peringatan (&deg;C)</label><input id="warn-th" type="number" step="0.1"></div>
<div><label>Ambang Alarm (&deg;C)</label><input id="stage2-th" type="number" step="0.1"></div>
<div><label>Kipas 1 Dasar</label><select id="fan1-baseline"><option value="true">NYALA</option><option value="false">MATI</option></select></div>
<div><label>Interval Sensor (detik)</label><input id="sensor-int" type="number" min="1"></div>
<div><label>Interval Cloud (detik)</label><input id="cloud-int" type="number" min="10"></div>
</div>
<div class="acts"><button class="btn bp" onclick="saveThermal()">Simpan Termal</button></div>
</div></div>
<div class="tp" id="tab-security"><div class="card">
<h2>Konfigurasi Keamanan <span class="tag">Akses &amp; Kunci</span></h2>
<div class="grid">
<div><label>Maks Percobaan Gagal</label><input id="max-fail" type="number" min="1"></div>
<div><label>Penguncian (detik)</label><input id="lockout-sec" type="number" min="10"></div>
<div><label>Buka Solenoid (detik)</label><input id="unlock-sec" type="number" min="1"></div>
<div><label>ID Perangkat</label><input id="device-id" type="text"></div>
</div>
<div class="acts"><button class="btn bp" onclick="saveSecurity()">Simpan Keamanan</button></div>
</div></div>
<div class="tp" id="tab-users"><div class="card">
<h2>Manajemen Pengguna <span class="tag">PIN Akses</span></h2>
<div class="grid">
<div><label>ID Pengguna</label><input id="u-id" type="text" placeholder="admin1"></div>
<div><label>Nama Tampilan</label><input id="u-name" type="text" placeholder="Admin 1"></div>
<div><label>PIN (4-8 digit)</label><input id="u-pin" type="password" placeholder="1234"></div>
</div>
<div class="acts"><button class="btn bp" onclick="saveUser()">Tambah / Ubah</button><button class="btn bs" onclick="loadUsers()">Muat Ulang</button></div>
<table><thead><tr><th>ID</th><th>Nama</th><th>Aktif</th><th>Aksi</th></tr></thead><tbody id="users-body"></tbody></table>
</div></div>
</div>
<div class="tbox" id="tbox"></div>
<script>
function sT(n){document.querySelectorAll('.tp').forEach(function(p){p.classList.remove('active')});document.querySelectorAll('.tb').forEach(function(b){b.classList.remove('active')});document.getElementById('tab-'+n).classList.add('active');document.querySelector('[data-tab="'+n+'"]').classList.add('active')}
function toast(m,ok){var t=document.createElement('div');t.className='tst '+(ok?'ok':'er');t.textContent=m;document.getElementById('tbox').appendChild(t);setTimeout(function(){t.style.opacity='0';t.style.transform='translateX(40px)';setTimeout(function(){t.remove()},300)},3500)}
var selectedSsid='';
async function fj(u,o){var r=await fetch(u,o||{});var t=await r.text();try{return JSON.parse(t)}catch(e){return{raw:t,ok:r.ok}}}
function sigBars(rssi){var s=rssi>=-50?4:rssi>=-60?3:rssi>=-70?2:1;var h='';for(var i=1;i<=4;i++){var ht=[4,7,10,14][i-1];var c=i<=s?'var(--gn)':'rgba(255,255,255,.1)';h+='<span style="height:'+ht+'px;background:'+c+'"></span>'}return h}
async function scanWifi(){toast('Memindai jaringan...',true);try{var d=await fj('/api/wifi/scan');var l=document.getElementById('wifi-list');l.innerHTML='';var nets=d.networks||[];if(!nets.length){l.innerHTML='<div class="wi"><span class="wn">Tidak ada jaringan terdeteksi</span></div>';return}nets.forEach(function(n){var div=document.createElement('div');div.className='wi';div.innerHTML='<span class="wn">'+n.ssid+(n.saved?' &check;':'')+'</span><span class="wm"><span class="sb">'+sigBars(n.rssi)+'</span>'+n.rssi+' dBm</span>';div.onclick=function(){selectedSsid=n.ssid;document.getElementById('wifi-ssid').value=n.ssid;document.querySelectorAll('.wi').forEach(function(x){x.classList.remove('active')});div.classList.add('active')};l.appendChild(div)});toast('Ditemukan '+nets.length+' jaringan',true)}catch(e){toast('Pindai gagal',false)}}
async function connectWifi(){var si=document.getElementById('wifi-ssid').value.trim();var ssid=si||selectedSsid;if(!ssid){toast('Pilih atau isi SSID',false);return}var pass=document.getElementById('wifi-pass').value;toast('Menghubungkan ke '+ssid+'...',true);var d=await fj('/api/wifi/connect',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:ssid,password:pass})});if(!d.success){toast(d.error||'Gagal terhubung',false);return}await waitForWifi(ssid)}
async function waitForWifi(ts){var st=Date.now();while(Date.now()-st<20000){await new Promise(function(r){setTimeout(r,1000)});try{var s=await fj('/api/state');if(s.wifiConnected){toast('Terhubung ke '+(s.ssid||ts)+(s.ip?' | IP '+s.ip:''),true);return}if(s.wifiState==='ap'){toast('Gagal. Periksa SSID/kata sandi.',false);return}}catch(e){}}toast('Timeout. Periksa serial monitor.',false)}
async function loadThermal(){var c=await fj('/api/config/thermal');document.getElementById('warn-th').value=c.warnThreshold??27;document.getElementById('stage2-th').value=c.stage2Threshold??28;document.getElementById('fan1-baseline').value=String(c.fan1BaselineOn??true);document.getElementById('sensor-int').value=c.sensorReadIntervalSec??5;document.getElementById('cloud-int').value=c.cloudSendIntervalSec??60}
async function saveThermal(){var r=await fetch('/api/config/thermal',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({warnThreshold:parseFloat(document.getElementById('warn-th').value),stage2Threshold:parseFloat(document.getElementById('stage2-th').value),fan1BaselineOn:document.getElementById('fan1-baseline').value==='true',sensorReadIntervalSec:parseInt(document.getElementById('sensor-int').value),cloudSendIntervalSec:parseInt(document.getElementById('cloud-int').value)})});toast(r.ok?'Konfigurasi termal tersimpan':'Gagal menyimpan',r.ok)}
async function loadSecurity(){var c=await fj('/api/config/security');document.getElementById('max-fail').value=c.maxFail??3;document.getElementById('lockout-sec').value=c.lockoutSecs??120;document.getElementById('unlock-sec').value=c.unlockSecs??10;document.getElementById('device-id').value=c.deviceId??''}
async function saveSecurity(){var r=await fetch('/api/config/security',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({maxFail:parseInt(document.getElementById('max-fail').value),lockoutSecs:parseInt(document.getElementById('lockout-sec').value),unlockSecs:parseInt(document.getElementById('unlock-sec').value),deviceId:document.getElementById('device-id').value})});toast(r.ok?'Konfigurasi keamanan tersimpan':'Gagal menyimpan',r.ok)}
async function loadUsers(){var d=await fj('/api/users');var b=document.getElementById('users-body');b.innerHTML='';(d.users||[]).forEach(function(u){var tr=document.createElement('tr');tr.innerHTML='<td>'+u.userId+'</td><td>'+(u.displayName||'-')+'</td><td><span style="color:'+(u.enabled?'var(--gn)':'var(--rd)')+';">&bull;</span> '+(u.enabled?'Ya':'Tidak')+'</td><td><button class="btn bd" onclick="deleteUser(\''+u.userId+'\')">Hapus</button></td>';b.appendChild(tr)})}
async function saveUser(){var p={userId:document.getElementById('u-id').value.trim(),displayName:document.getElementById('u-name').value.trim(),pin:document.getElementById('u-pin').value.trim(),enabled:true};var r=await fetch('/api/users',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(p)});if(r.ok){toast('Pengguna tersimpan',true);document.getElementById('u-pin').value='';loadUsers()}else{var e=await r.text();toast('Gagal: '+e,false)}}
async function deleteUser(id){if(!confirm('Hapus pengguna '+id+'?'))return;var r=await fetch('/api/users/'+encodeURIComponent(id),{method:'DELETE'});if(r.ok){toast('Pengguna dihapus',true);loadUsers()}else{var e=await r.text();toast('Gagal: '+e,false)}}
scanWifi();loadThermal();loadSecurity();loadUsers();
</script>
</body>
</html>
)rawliteral";

constexpr const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>Dashboard — Smart Server</title>
<style>
:root{--bg:#060a13;--sf:rgba(12,17,32,.82);--bd:rgba(255,255,255,.07);--bh:rgba(255,255,255,.14);--tx:#edf0f7;--td:#6b7a99;--bl:#5b9aff;--cy:#22d3ee;--gn:#34d399;--rd:#f87171;--am:#fbbf24;--r:14px;--rs:10px}
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--tx);min-height:100vh;background-image:radial-gradient(ellipse at 15% 5%,rgba(91,154,255,.1),transparent 55%),radial-gradient(ellipse at 85% 95%,rgba(34,211,238,.07),transparent 55%);background-attachment:fixed}
.app{max-width:900px;margin:0 auto;padding:20px 16px 48px;display:flex;flex-direction:column;gap:16px}
.card{background:var(--sf);backdrop-filter:blur(24px);-webkit-backdrop-filter:blur(24px);border:1px solid var(--bd);border-radius:var(--r);padding:24px;transition:border-color .3s,box-shadow .3s}
.hdr{display:flex;justify-content:space-between;align-items:center;padding:18px 24px}
.bt{font-size:.6rem;font-weight:700;text-transform:uppercase;letter-spacing:.2em;color:var(--bl);background:rgba(91,154,255,.1);padding:3px 10px;border-radius:6px;margin-bottom:4px;display:inline-block}
h1{font-size:1.35rem;font-weight:800;letter-spacing:-.02em;color:#fff}
.grd{background:linear-gradient(135deg,var(--bl),var(--cy));-webkit-background-clip:text;-webkit-text-fill-color:transparent}
.conn{display:flex;align-items:center;gap:8px;font-size:.82rem;color:var(--td);font-weight:500;background:rgba(0,0,0,.25);padding:8px 14px;border-radius:var(--rs);border:1px solid var(--bd)}
.dot{width:8px;height:8px;border-radius:50%;background:var(--am);flex-shrink:0;animation:pA 2s infinite}
.dot.on{background:var(--gn);animation:pG 2s infinite}
.dot.off{background:var(--rd);animation:none}
@keyframes pA{0%,100%{box-shadow:0 0 0 0 rgba(251,191,36,.4)}50%{box-shadow:0 0 0 6px rgba(251,191,36,0)}}
@keyframes pG{0%,100%{box-shadow:0 0 0 0 rgba(52,211,153,.4)}50%{box-shadow:0 0 0 6px rgba(52,211,153,0)}}
.alrt{padding:14px 20px;border-radius:var(--r);display:flex;align-items:center;gap:12px;font-size:.88rem;font-weight:600;animation:fi .35s ease}
.alrt.w{background:rgba(251,191,36,.08);border:1px solid rgba(251,191,36,.25);color:var(--am)}
.alrt.d{background:rgba(248,113,113,.08);border:1px solid rgba(248,113,113,.3);color:var(--rd);animation:fi .35s ease,bk 1.5s infinite}
@keyframes bk{0%,100%{opacity:1}50%{opacity:.65}}
@keyframes fi{from{opacity:0;transform:translateY(8px)}to{opacity:1;transform:translateY(0)}}
.metrics{display:grid;grid-template-columns:repeat(3,1fr);gap:14px}
.mc{background:var(--sf);backdrop-filter:blur(24px);border:1px solid var(--bd);border-radius:var(--r);padding:20px;display:flex;align-items:center;gap:16px;transition:border-color .3s,box-shadow .3s;position:relative;overflow:hidden;animation:fi .5s ease both}
.mc:nth-child(2){animation-delay:.06s}
.mc:nth-child(3){animation-delay:.12s}
.mc::after{content:'';position:absolute;top:-20px;right:-20px;width:80px;height:80px;border-radius:50%;filter:blur(40px);opacity:.12;pointer-events:none}
.mc.t::after{background:var(--bl)}
.mc.h::after{background:var(--cy)}
.mc.w::after{background:var(--gn)}
.mi{width:44px;height:44px;border-radius:12px;display:flex;align-items:center;justify-content:center;flex-shrink:0}
.ml{font-size:.7rem;color:var(--td);text-transform:uppercase;letter-spacing:.06em;font-weight:600;margin-bottom:2px}
.mr{display:flex;align-items:baseline;gap:3px}
.mv{font-size:2rem;font-weight:800;color:#fff;line-height:1;font-variant-numeric:tabular-nums;transition:color .4s}
.mu{font-size:.85rem;color:var(--td);font-weight:600}
.mc.warn{border-color:rgba(251,191,36,.35);box-shadow:0 0 20px -4px rgba(251,191,36,.2)}
.mc.alarm{border-color:rgba(248,113,113,.4);box-shadow:0 0 24px -4px rgba(248,113,113,.25);animation:gR 1.5s ease-in-out infinite}
@keyframes gR{0%,100%{box-shadow:0 0 20px -4px rgba(248,113,113,.2)}50%{box-shadow:0 0 32px -2px rgba(248,113,113,.35)}}
.panels{display:grid;grid-template-columns:1fr 1fr;gap:14px}
.panel{animation:fi .5s ease both}
.panels .panel:nth-child(2){animation-delay:.1s}
.ph{font-size:.88rem;font-weight:700;color:#fff;margin-bottom:14px;display:flex;align-items:center;gap:8px}
.ph .pi{width:28px;height:28px;border-radius:8px;display:flex;align-items:center;justify-content:center;font-size:.85rem}
.rows{display:flex;flex-direction:column;gap:9px}
.rw{display:flex;justify-content:space-between;align-items:center}
.rk{font-size:.8rem;color:var(--td);font-weight:500}
.bg{font-size:.68rem;font-weight:700;padding:3px 10px;border-radius:16px;text-transform:uppercase;letter-spacing:.03em}
.bg-on{background:rgba(52,211,153,.12);color:var(--gn)}
.bg-off{background:rgba(255,255,255,.05);color:var(--td)}
.bg-al{background:rgba(248,113,113,.12);color:var(--rd)}
.bg-lk{background:rgba(91,154,255,.12);color:var(--bl)}
.bg-ul{background:rgba(251,191,36,.12);color:var(--am)}
.bg-mt{background:rgba(255,255,255,.05);color:var(--td)}
.ic{background:rgba(0,0,0,.2);border-radius:var(--rs);padding:14px 18px;border:1px solid var(--bd);animation:fi .5s ease .15s both}
.il{font-size:.68rem;color:var(--td);font-weight:600;text-transform:uppercase;letter-spacing:.05em;margin-bottom:8px}
.iv{font-family:ui-monospace,SFMono-Regular,Consolas,monospace;font-size:.82rem;color:var(--cy);word-break:break-all;line-height:1.7}
.acts{display:flex;gap:10px;flex-wrap:wrap;animation:fi .5s ease .2s both}
.btn{padding:10px 20px;border-radius:var(--rs);border:none;font-weight:600;font-size:.82rem;cursor:pointer;transition:all .2s;display:inline-flex;align-items:center;gap:6px;text-decoration:none;font-family:inherit}
.bp{background:linear-gradient(135deg,var(--bl),#3d78e0);color:#fff;box-shadow:0 4px 14px rgba(91,154,255,.25)}
.bp:hover{transform:translateY(-1px);box-shadow:0 6px 20px rgba(91,154,255,.35)}
.bs{background:rgba(255,255,255,.05);color:var(--tx);border:1px solid var(--bd)}
.bs:hover{background:rgba(255,255,255,.08);border-color:var(--bh)}
.bd{background:linear-gradient(135deg,var(--rd),#dc2626);color:#fff;box-shadow:0 4px 14px rgba(248,113,113,.2)}
.bd:hover{box-shadow:0 6px 20px rgba(248,113,113,.3);transform:translateY(-1px)}
.btn:active{transform:translateY(0)!important}
.ft{text-align:center;font-size:.68rem;color:var(--td);padding-top:4px}
@media(max-width:700px){.app{padding:14px 10px 32px}.hdr{flex-direction:column;align-items:flex-start;gap:10px;padding:16px}.metrics{grid-template-columns:1fr}.panels{grid-template-columns:1fr}.mv{font-size:1.7rem}.acts{flex-direction:column}.acts .btn{width:100%;justify-content:center}}
</style>
</head>
<body>
<div class="app">
<header class="card hdr">
<div><div class="bt">Smart Server</div><h1>Dashboard <span class="grd">Cloud</span></h1></div>
<div class="conn"><span class="dot" id="dot"></span><span id="ct">Menghubungkan...</span></div>
</header>
<div class="alrt" id="alrt" style="display:none"><span id="ai"></span><span id="am"></span></div>
<div class="metrics">
<div class="mc t" id="c-temp">
<div class="mi" style="background:rgba(91,154,255,.12);color:var(--bl)"><svg viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg></div>
<div><div class="ml">Suhu</div><div class="mr"><span class="mv" id="temp">--</span><span class="mu">&deg;C</span></div></div>
</div>
<div class="mc h">
<div class="mi" style="background:rgba(34,211,238,.12);color:var(--cy)"><svg viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/></svg></div>
<div><div class="ml">Kelembapan</div><div class="mr"><span class="mv" id="hum">--</span><span class="mu">%</span></div></div>
</div>
<div class="mc w">
<div class="mi" style="background:rgba(52,211,153,.12);color:var(--gn)"><svg viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><path d="M5 12.55a11 11 0 0 1 14.08 0"/><path d="M1.42 9a16 16 0 0 1 21.16 0"/><path d="M8.53 16.11a6 6 0 0 1 6.95 0"/><circle cx="12" cy="20" r="1" fill="currentColor"/></svg></div>
<div><div class="ml">Sinyal WiFi</div><div class="mr"><span class="mv" id="rssi">--</span><span class="mu">dBm</span></div></div>
</div>
</div>
<div class="panels">
<div class="card panel">
<div class="ph"><span class="pi" style="background:rgba(91,154,255,.1)">&#9881;</span> Status Perangkat</div>
<div class="rows">
<div class="rw"><span class="rk">Kipas 1</span><span class="bg bg-mt" id="fan1">--</span></div>
<div class="rw"><span class="rk">Kipas 2</span><span class="bg bg-mt" id="fan2">--</span></div>
<div class="rw"><span class="rk">Alarm</span><span class="bg bg-mt" id="alarm">--</span></div>
<div class="rw"><span class="rk">Pintu</span><span class="bg bg-mt" id="door">--</span></div>
<div class="rw"><span class="rk">Antrean Cloud</span><span class="bg bg-mt" id="queue">--</span></div>
</div>
</div>
<div class="card panel">
<div class="ph"><span class="pi" style="background:rgba(251,191,36,.1)">&#128274;</span> Keamanan</div>
<div class="rows">
<div class="rw"><span class="rk">WiFi</span><span class="bg bg-mt" id="wifi">--</span></div>
<div class="rw"><span class="rk">SSID</span><span class="bg bg-mt" id="ssid">--</span></div>
<div class="rw"><span class="rk">Terkunci</span><span class="bg bg-mt" id="lock">--</span></div>
<div class="rw"><span class="rk">Gagal</span><span class="bg bg-mt" id="fail">--</span></div>
<div class="rw"><span class="rk">Pesan</span><span class="bg bg-mt" id="msg">--</span></div>
</div>
</div>
</div>
<div class="ic" id="info-card"><div class="il">Akses Lokal</div><div class="iv" id="local">Memuat...</div></div>
<div class="acts">
<button class="btn bp" onclick="sendNow()">&#9889; Kirim Sekarang</button>
<a class="btn bs" href="/setup">&#9881; Pengaturan</a>
<button class="btn bd" onclick="formatFlash()">&#9888; Format Flash</button>
</div>
<div class="ft" id="ft"></div>
</div>
<script>
var tc=document.getElementById('c-temp');
function setDot(on){var d=document.getElementById('dot');d.classList.toggle('on',on);d.classList.toggle('off',!on);document.getElementById('ct').textContent=on?'Terhubung':'Terputus'}
function setBg(el,cls,txt){el.className='bg '+cls;el.textContent=txt}
async function refresh(){try{var r=await fetch('/api/state');var d=await r.json();document.getElementById('temp').textContent=d.valid?d.temperature.toFixed(1):'--';document.getElementById('hum').textContent=d.valid?d.humidity.toFixed(1):'--';document.getElementById('rssi').textContent=d.wifiConnected?d.rssi:'--';
setBg(document.getElementById('fan1'),d.fan1On?'bg-on':'bg-off',d.fan1On?'ON':'OFF');
setBg(document.getElementById('fan2'),d.fan2On?'bg-on':'bg-off',d.fan2On?'ON':'OFF');
setBg(document.getElementById('alarm'),d.alarm?'bg-al':'bg-on',d.alarm?'ALARM':'NORMAL');
setBg(document.getElementById('door'),d.doorState==='LOCKED'?'bg-lk':'bg-ul',d.doorState);
setBg(document.getElementById('queue'),'bg-mt',d.queueTelemetry+' / '+d.queueAccess);
setBg(document.getElementById('wifi'),d.wifiConnected?'bg-on':'bg-off',d.wifiConnected?'ON':'OFF');
setBg(document.getElementById('ssid'),'bg-mt',d.ssid||'-');
setBg(document.getElementById('lock'),d.lockoutActive?'bg-al':'bg-on',d.lockoutActive?'YA '+d.lockoutRemainingSec+'d':'TIDAK');
setBg(document.getElementById('fail'),'bg-mt',String(d.failedAttempts));
setBg(document.getElementById('msg'),'bg-mt',d.accessMessage||'-');
tc.classList.remove('warn','alarm');if(d.alarm){tc.classList.add('alarm')}else if(d.alertOn){tc.classList.add('warn')}
var al=document.getElementById('alrt');if(d.alarm){al.style.display='flex';al.className='alrt d';document.getElementById('ai').textContent='\uD83D\uDEA8';document.getElementById('am').textContent='ALARM \u2014 Suhu kritis! '+( d.valid?d.temperature.toFixed(1)+'\u00B0C':'')}else if(d.alertOn){al.style.display='flex';al.className='alrt w';document.getElementById('ai').textContent='\u26A0\uFE0F';document.getElementById('am').textContent='Peringatan \u2014 Suhu tinggi '+(d.valid?d.temperature.toFixed(1)+'\u00B0C':'')}else{al.style.display='none'}
var info=d.wifiConnected?'http://'+d.ip+'/':'Hubungkan ESP ke WiFi untuk akses lokal.';if(d.wifiConnected&&d.mdns)info+='\n'+' http://'+d.mdns+'/';document.getElementById('local').textContent=info;
setDot(d.wifiConnected);
var ls='';if(d.lastSend>1000){var dt=new Date(d.lastSend*1000);ls=' \u00B7 Terkirim: '+dt.toLocaleTimeString('id-ID')}document.getElementById('ft').textContent='Device: '+(d.deviceId||'-')+' \u00B7 '+d.wifiState+ls;
}catch(e){setDot(false);document.getElementById('ft').textContent='Kesalahan koneksi'}}
async function sendNow(){await fetch('/api/send',{method:'POST'});refresh()}
async function formatFlash(){if(!confirm('Format flash akan menghapus config WiFi, PIN, dan data tersimpan. Lanjutkan?'))return;var r=await fetch('/api/flash/format',{method:'POST'});var d=await r.json().catch(function(){return{}});if(!r.ok){alert(d.error||'Gagal format flash');return}alert('Flash diformat. ESP akan restart.')}
refresh();setInterval(refresh,3000);
</script>
</body>
</html>
)rawliteral";

}  // namespace WebPage
