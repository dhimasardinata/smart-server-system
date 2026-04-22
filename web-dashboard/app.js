// Dashboard ini membaca data dari Google Sheets, lalu menampilkannya
// dalam bentuk kartu ringkas, tabel, grafik, dan notifikasi.
// Kunci di bawah ini dipakai untuk menyimpan setelan di browser.
const CONFIG_KEY = 'smart-server-sheet-id';
const DEFAULT_SHEET_ID = '1rKF8ZZWsYCXlh_yixmdfKyZA6omkj64F5pEjiMcfAw8';
const REFRESH_INTERVAL = 15000;
const NOTIF_KEY = 'smart-server-notif';
const PAGE_SIZE = 10;

// Semua elemen halaman disimpan di satu tempat supaya mudah dipakai ulang.
const el = {
    statusDot: document.getElementById('status-dot'),
    statusText: document.getElementById('status-text'),
    refreshBtn: document.getElementById('refresh-btn'),
    tempValue: document.getElementById('temp-value'),
    humValue: document.getElementById('hum-value'),
    rssiValue: document.getElementById('rssi-value'),
    tempThreshold: document.getElementById('temp-threshold'),
    humThreshold: document.getElementById('hum-threshold'),
    rssiQuality: document.getElementById('rssi-quality'),
    tempCard: document.getElementById('temp-card'),
    humCard: document.getElementById('hum-card'),
    alarmState: document.getElementById('alarm-state'),
    fan1State: document.getElementById('fan1-state'),
    fan2State: document.getElementById('fan2-state'),
    doorState: document.getElementById('door-state'),
    grantedCount: document.getElementById('granted-count'),
    deniedCount: document.getElementById('denied-count'),
    lockoutCount: document.getElementById('lockout-count'),
    lastUpdate: document.getElementById('last-update'),
    telemetryBody: document.getElementById('telemetry-body'),
    accessBody: document.getElementById('access-body'),
    telemetryPagination: document.getElementById('telemetry-pagination'),
    accessPagination: document.getElementById('access-pagination'),
    telemetryInfo: document.getElementById('telemetry-info'),
    accessInfo: document.getElementById('access-info'),
    sheetId: document.getElementById('sheet-id'),
    notifToggle: document.getElementById('notif-toggle'),
    saveBtn: document.getElementById('save-btn'),
    toastContainer: document.getElementById('toast-container'),
    thWarn: document.getElementById('th-warn'),
    thAlarm: document.getElementById('th-alarm'),
    thHumWarn: document.getElementById('th-hum-warn'),
    thHumAlarm: document.getElementById('th-hum-alarm'),
    thStatus: document.getElementById('th-status'),
    alertBanner: document.getElementById('alert-banner'),
    alertIcon: document.getElementById('alert-icon'),
    alertTitle: document.getElementById('alert-title'),
    alertMsg: document.getElementById('alert-msg')
};

let trendChart = null;
let telemetryData = [];
let accessData = [];
let telemetryPage = 1;
let accessPage = 1;

function initChart() {
    // Grafik dibuat sekali saat halaman mulai agar update berikutnya cepat.
    // Setelah itu, yang berubah hanya isi datanya.
    const ctx = document.getElementById('trend-chart').getContext('2d');

    // Warna gradasi untuk garis suhu.
    const tempGrad = ctx.createLinearGradient(0, 0, 0, 320);
    tempGrad.addColorStop(0, 'rgba(79, 143, 255, 0.3)');
    tempGrad.addColorStop(1, 'rgba(79, 143, 255, 0.0)');

    // Warna gradasi untuk garis kelembapan.
    const humGrad = ctx.createLinearGradient(0, 0, 0, 320);
    humGrad.addColorStop(0, 'rgba(34, 211, 238, 0.25)');
    humGrad.addColorStop(1, 'rgba(34, 211, 238, 0.0)');

    trendChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                {
                    label: 'Suhu (°C)',
                    data: [],
                    borderColor: '#4f8fff',
                    backgroundColor: tempGrad,
                    fill: true,
                    tension: 0.4,
                    pointRadius: 0,
                    pointHoverRadius: 5,
                    borderWidth: 2
                },
                {
                    label: 'Kelembapan (%)',
                    data: [],
                    borderColor: '#22d3ee',
                    backgroundColor: humGrad,
                    fill: true,
                    tension: 0.4,
                    pointRadius: 0,
                    pointHoverRadius: 5,
                    borderWidth: 2
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            interaction: { mode: 'index', intersect: false },
            scales: {
                x: {
                    ticks: { color: '#7c86a2', maxRotation: 0, autoSkipPadding: 24, font: { size: 11 } },
                    grid: { display: false }
                },
                y: {
                    ticks: { color: '#7c86a2', font: { size: 11 } },
                    grid: { color: 'rgba(255, 255, 255, 0.04)', borderDash: [4, 4] },
                    beginAtZero: false
                }
            },
            plugins: {
                legend: {
                    labels: { color: '#eef0f6', usePointStyle: true, boxWidth: 6, font: { size: 11, weight: '600' } },
                    position: 'top',
                    align: 'end'
                },
                tooltip: {
                    backgroundColor: 'rgba(6, 8, 15, 0.92)',
                    titleColor: '#eef0f6',
                    bodyColor: '#9ca3b8',
                    borderColor: 'rgba(255,255,255,0.08)',
                    borderWidth: 1,
                    padding: 10,
                    boxPadding: 4,
                    usePointStyle: true,
                    cornerRadius: 8
                }
            }
        }
    });
}

function loadConfig() {
    // Kalau pernah simpan ID spreadsheet, ambil lagi dari localStorage.
    // Kalau belum ada, pakai ID bawaan sebagai contoh awal.
    const savedSheet = localStorage.getItem(CONFIG_KEY);
    el.sheetId.value = savedSheet || DEFAULT_SHEET_ID;
    el.notifToggle.checked = localStorage.getItem(NOTIF_KEY) === 'true';
    return el.sheetId.value;
}

function saveConfig() {
    // Ambil nilai spreadsheet dari kotak input.
    // Nilai ini disimpan di browser, bukan di server.
    const value = (el.sheetId.value || '').trim();
    if (!value) return null;
    // Simpan pilihan ke browser agar tidak hilang saat halaman ditutup.
    localStorage.setItem(CONFIG_KEY, value);
    localStorage.setItem(NOTIF_KEY, el.notifToggle.checked);
    showToast('Terhubung', 'ID Spreadsheet tersimpan', 'success');
    return value;
}

function showToast(title, message, type = 'warning') {
    // Toast adalah pop-up kecil untuk memberi informasi cepat.
    // Ini dipakai supaya pengguna langsung tahu hasil aksi.
    if (!el.notifToggle.checked && type !== 'success') return;

    const toast = document.createElement('div');
    toast.className = `toast ${type}`;

    const icons = { danger: '🚨', success: '✅', warning: '🔔' };
    toast.innerHTML = `
        <div class="toast-icon">${icons[type] || '🔔'}</div>
        <div class="toast-content">
            <div class="toast-title">${title}</div>
            <div class="toast-msg">${message}</div>
        </div>
    `;

    toast.onclick = () => {
        toast.classList.add('hiding');
        setTimeout(() => toast.remove(), 350);
    };

    el.toastContainer.appendChild(toast);
    setTimeout(() => {
        if (toast.parentElement) {
            toast.classList.add('hiding');
            setTimeout(() => toast.remove(), 350);
        }
    }, 5000);
}

function parseGvizDate(value) {
    // Tanggal dari spreadsheet bisa datang dalam beberapa bentuk.
    if (value instanceof Date) return value;
    if (typeof value === 'string' && value.includes('Date(')) {
        const m = value.match(/Date\((\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\)/);
        if (m) return new Date(+m[1], +m[2], +m[3], +m[4], +m[5], +m[6]);
    }
    if (typeof value === 'string') {
        const dt = new Date(value);
        if (!Number.isNaN(dt.getTime())) return dt;
    }
    return null;
}

async function fetchSheet(sheetId, sheetName) {
    // Ambil data dari spreadsheet lewat alamat publik gviz.
    // Hasil mentah ini nanti dipilah lagi supaya cocok dengan tampilan.
    const url = `https://docs.google.com/spreadsheets/d/${sheetId}/gviz/tq?tqx=out:json&sheet=${encodeURIComponent(sheetName)}`;
    const response = await fetch(url);
    const text = await response.text();
    const jsonMatch = text.match(/google\.visualization\.Query\.setResponse\(([\s\S]*)\);?$/);
    if (!jsonMatch) throw new Error(`Respons tidak valid untuk ${sheetName}`);
    return JSON.parse(jsonMatch[1]);
}

function parseTelemetry(gviz) {
    // Data dari spreadsheet diubah menjadi format yang mudah dipakai halaman ini.
    // Bagian ini menyesuaikan bentuk data mentah agar cocok dengan tabel.
    return (gviz?.table?.rows || []).map(r => {
        // Ambil kolom satu per satu dari baris spreadsheet.
        const c = r.c || [];
        const temp = Number(c[2]?.v);
        const hum = Number(c[3]?.v);
        const rssi = Number(c[8]?.v);
        return {
            timestamp: parseGvizDate(c[0]?.v),
            deviceId: String(c[1]?.v || ''),
            temperature: isFinite(temp) ? temp : 0,
            humidity: isFinite(hum) ? hum : 0,
            fan1On: String(c[4]?.v || '').toLowerCase() === 'true',
            fan2On: String(c[5]?.v || '').toLowerCase() === 'true',
            alarmState: String(c[6]?.v || 'NORMAL'),
            doorState: String(c[7]?.v || 'LOCKED'),
            wifiRssi: isFinite(rssi) ? rssi : 0,
            warnThreshold: c[9]?.v != null ? Number(c[9].v) : null,
            stage2Threshold: c[10]?.v != null ? Number(c[10].v) : null,
            warnHumThreshold: c[11]?.v != null ? Number(c[11].v) : null,
            stage2HumThreshold: c[12]?.v != null ? Number(c[12].v) : null
        };
    }).filter(x => x.timestamp);
}

function parseAccess(gviz) {
    // Data akses juga dibaca dari lembar khususnya sendiri.
    return (gviz?.table?.rows || []).map(r => {
        // Ambil data akses dari kolom yang sesuai.
        const c = r.c || [];
        return {
            timestamp: parseGvizDate(c[0]?.v),
            deviceId: String(c[1]?.v || ''),
            userId: String(c[2]?.v || ''),
            displayName: String(c[3]?.v || ''),
            result: String(c[4]?.v || ''),
            reason: String(c[5]?.v || ''),
            failedCount: Number(c[6]?.v || 0),
            lockoutUntil: String(c[7]?.v || ''),
            doorState: String(c[8]?.v || '')
        };
    }).filter(x => x.timestamp);
}

function fmtTime(dt) {
    if (!dt) return '--';
    return dt.toLocaleString('id-ID', {
        day: '2-digit', month: 'short',
        hour: '2-digit', minute: '2-digit', second: '2-digit'
    });
}

function setStatus(online, text) {
    // Lampu kecil ini menandakan apakah data berhasil dimuat.
    el.statusDot.classList.toggle('online', online);
    el.statusDot.classList.toggle('offline', !online);
    el.statusText.textContent = text;
}

function badgeClass(type, value) {
    // Pilih warna label kecil sesuai keadaan data.
    const map = {
        fan: value ? 'badge-on' : 'badge-off',
        alarm: value === 'ALARM' ? 'badge-alarm' : 'badge-normal',
        door: value === 'LOCKED' ? 'badge-locked' : 'badge-unlocked',
        result: { GRANTED: 'badge-granted', DENIED: 'badge-denied', LOCKOUT: 'badge-lockout' }[value] || ''
    };
    return map[type] || map;
}

function rssiQuality(rssi) {
    // Ubah nilai sinyal jadi kata yang mudah dibaca.
    if (rssi >= -50) return { text: 'Sangat Baik', cls: 'badge-success' };
    if (rssi >= -60) return { text: 'Baik', cls: 'badge-success' };
    if (rssi >= -70) return { text: 'Cukup', cls: 'badge-warning' };
    return { text: 'Lemah', cls: 'badge-danger' };
}

function safeFixed(v) { return isFinite(v) ? v.toFixed(1) : '--'; }

function renderPagination(container, totalItems, currentPage, onPageChange) {
    // Hitung jumlah halaman yang perlu ditampilkan.
    // Tombol dibuat sedikit saja supaya tidak memenuhi layar.
    const totalPages = Math.max(1, Math.ceil(totalItems / PAGE_SIZE));
    if (currentPage > totalPages) currentPage = totalPages;

    let html = `<button ${currentPage <= 1 ? 'disabled' : ''} data-page="${currentPage - 1}">‹</button>`;

    const maxButtons = 5;
    let start = Math.max(1, currentPage - Math.floor(maxButtons / 2));
    let end = Math.min(totalPages, start + maxButtons - 1);
    if (end - start < maxButtons - 1) start = Math.max(1, end - maxButtons + 1);

    for (let i = start; i <= end; i++) {
        html += `<button class="${i === currentPage ? 'active' : ''}" data-page="${i}">${i}</button>`;
    }

    html += `<button ${currentPage >= totalPages ? 'disabled' : ''} data-page="${currentPage + 1}">›</button>`;
    html += `<span class="page-info">${totalItems} data</span>`;

    container.innerHTML = html;
    container.querySelectorAll('button[data-page]').forEach(btn => {
        btn.addEventListener('click', () => {
            const page = parseInt(btn.dataset.page);
            if (page >= 1 && page <= totalPages) onPageChange(page);
        });
    });
}

function renderTelemetryTable(items, page) {
    // Tabel pantauan menampilkan data terbaru di bagian atas.
    // Urutkan dari data paling baru ke paling lama.
    // Dengan begitu, data terkini selalu muncul duluan.
    const allSorted = items.slice().reverse();
    const totalPages = Math.max(1, Math.ceil(allSorted.length / PAGE_SIZE));
    if (page > totalPages) page = totalPages;
    const start = (page - 1) * PAGE_SIZE;
    const pageItems = allSorted.slice(start, start + PAGE_SIZE);

    el.telemetryInfo.textContent = `Hal ${page}/${totalPages}`;

    el.telemetryBody.innerHTML = pageItems.map(x => {
        const fan1Cls = badgeClass('fan', x.fan1On);
        const fan2Cls = badgeClass('fan', x.fan2On);
        const alarmCls = badgeClass('alarm', x.alarmState);
        const doorCls = badgeClass('door', x.doorState);
        return `<tr>
            <td>${fmtTime(x.timestamp)}</td>
            <td>${safeFixed(x.temperature)}</td>
            <td>${safeFixed(x.humidity)}</td>
            <td><span class="badge ${fan1Cls}">${x.fan1On ? 'ON' : 'OFF'}</span></td>
            <td><span class="badge ${fan2Cls}">${x.fan2On ? 'ON' : 'OFF'}</span></td>
            <td><span class="badge ${alarmCls}">${x.alarmState}</span></td>
            <td><span class="badge ${doorCls}">${x.doorState}</span></td>
        </tr>`;
    }).join('');

    renderPagination(el.telemetryPagination, allSorted.length, page, (p) => {
        telemetryPage = p;
        renderTelemetryTable(telemetryData, p);
    });
}

function renderAccessTable(items, page) {
    // Tabel akses juga diurutkan dari data paling baru.
    // Urutkan dari data paling baru ke paling lama.
    // Jadi kejadian terakhir langsung terlihat.
    const allSorted = items.slice().reverse();
    const totalPages = Math.max(1, Math.ceil(allSorted.length / PAGE_SIZE));
    if (page > totalPages) page = totalPages;
    const start = (page - 1) * PAGE_SIZE;
    const pageItems = allSorted.slice(start, start + PAGE_SIZE);

    el.accessInfo.textContent = `Hal ${page}/${totalPages}`;

    el.accessBody.innerHTML = pageItems.map(x => {
        const resCls = badgeClass('result', x.result);
        return `<tr>
            <td>${fmtTime(x.timestamp)}</td>
            <td>${x.userId || '-'}</td>
            <td>${x.displayName || '-'}</td>
            <td><span class="badge ${resCls}">${x.result || '-'}</span></td>
            <td>${x.reason || '-'}</td>
            <td>${x.failedCount}</td>
            <td>${x.doorState || '-'}</td>
        </tr>`;
    }).join('');

    renderPagination(el.accessPagination, allSorted.length, page, (p) => {
        accessPage = p;
        renderAccessTable(accessData, p);
    });
}

function updateAlertState(latest) {
    // Kalau data melewati ambang, tampilkan peringatan yang sesuai.
    // Bersihkan kelas lama dulu supaya tampilan tidak numpuk.
    // Setelah itu, warna dan pesan diatur ulang dari awal.
    el.tempCard.classList.remove('card-warning', 'card-alarm');
    el.humCard.classList.remove('card-warning', 'card-alarm');

    const hasTempTh = latest.warnThreshold != null && isFinite(latest.warnThreshold);
    const hasHumTh = latest.warnHumThreshold != null && isFinite(latest.warnHumThreshold);
    const isAlarm = latest.alarmState === 'ALARM';

    const tempWarning = hasTempTh && latest.temperature > latest.warnThreshold;
    const tempCritical = hasTempTh && latest.temperature > latest.stage2Threshold;
    const humWarning = hasHumTh && latest.humidity > latest.warnHumThreshold;
    const humCritical = hasHumTh && latest.humidity > latest.stage2HumThreshold;

    const anyCritical = isAlarm || tempCritical || humCritical;
    const anyWarning = (tempWarning || humWarning) && !anyCritical;

    if (tempCritical || (isAlarm && !humCritical)) el.tempCard.classList.add('card-alarm');
    else if (tempWarning) el.tempCard.classList.add('card-warning');

    if (humCritical) el.humCard.classList.add('card-alarm');
    else if (humWarning) el.humCard.classList.add('card-warning');

    if (anyCritical) {
        const reasons = [];
        if (tempCritical || isAlarm) reasons.push(`Suhu ${safeFixed(latest.temperature)}°C > ${latest.stage2Threshold || '--'}°C`);
        if (humCritical) reasons.push(`Kelembapan ${safeFixed(latest.humidity)}% > ${latest.stage2HumThreshold || '--'}%`);
        el.alertBanner.style.display = 'flex';
        el.alertBanner.className = 'alert-banner alert-danger';
        el.alertIcon.textContent = '🚨';
        el.alertTitle.textContent = 'ALARM — Kondisi Kritis!';
        el.alertMsg.textContent = reasons.join(' · ') + '. Kedua kipas aktif.';
        showToast('Alarm Lingkungan', reasons.join(', '), 'danger');
    } else if (anyWarning) {
        const reasons = [];
        if (tempWarning) reasons.push(`Suhu ${safeFixed(latest.temperature)}°C > ${latest.warnThreshold}°C`);
        if (humWarning) reasons.push(`Kelembapan ${safeFixed(latest.humidity)}% > ${latest.warnHumThreshold}%`);
        el.alertBanner.style.display = 'flex';
        el.alertBanner.className = 'alert-banner alert-warning';
        el.alertIcon.textContent = '⚠️';
        el.alertTitle.textContent = 'Peringatan — Kondisi Tinggi';
        el.alertMsg.textContent = reasons.join(' · ');
    } else {
        el.alertBanner.style.display = 'none';
    }
}

function updateSummary(telemetry, access) {
    // Ringkasan di layar utama diambil dari data terbaru saja.
    // Kalau belum ada data, tampilannya sengaja dibuat kosong.
    if (telemetry.length === 0) {
        // Kalau belum ada data, tampilkan tanda kosong.
        el.tempValue.textContent = '--';
        el.humValue.textContent = '--';
        el.rssiValue.textContent = '--';
        return;
    }

    const latest = telemetry[telemetry.length - 1];

    // Tampilkan suhu dan kelembapan terbaru.
    el.tempValue.textContent = safeFixed(latest.temperature);
    el.humValue.textContent = safeFixed(latest.humidity);

    if (latest.wifiRssi !== 0) {
        // Tampilkan kualitas sinyal kalau nilainya ada.
        el.rssiValue.textContent = latest.wifiRssi;
        const rq = rssiQuality(latest.wifiRssi);
        el.rssiQuality.innerHTML = `<span class="badge ${rq.cls}">${rq.text}</span>`;
    } else {
        el.rssiValue.textContent = '--';
        el.rssiQuality.textContent = '';
    }

    if (latest.warnThreshold != null && isFinite(latest.warnThreshold)) {
        // Tampilkan ambang batas yang dipakai ESP32.
        el.tempThreshold.textContent = `Peringatan: ${latest.warnThreshold}°C · Alarm: ${latest.stage2Threshold}°C`;
        el.humThreshold.textContent = `Peringatan: ${latest.warnHumThreshold ?? '--'}% · Alarm: ${latest.stage2HumThreshold ?? '--'}%`;
        el.thWarn.textContent = `${latest.warnThreshold}°C`;
        el.thWarn.className = 'badge badge-warning';
        el.thAlarm.textContent = `${latest.stage2Threshold}°C`;
        el.thAlarm.className = 'badge badge-danger';
        el.thHumWarn.textContent = `${latest.warnHumThreshold ?? '--'}%`;
        el.thHumWarn.className = 'badge badge-warning';
        el.thHumAlarm.textContent = `${latest.stage2HumThreshold ?? '--'}%`;
        el.thHumAlarm.className = 'badge badge-danger';
        el.thStatus.textContent = 'Tersinkron';
        el.thStatus.className = 'badge badge-success';
    } else {
        el.tempThreshold.textContent = 'Ambang batas: menunggu sinkron ESP';
        el.humThreshold.textContent = '';
        el.thWarn.textContent = '--';
        el.thWarn.className = 'badge';
        el.thAlarm.textContent = '--';
        el.thAlarm.className = 'badge';
        el.thHumWarn.textContent = '--';
        el.thHumWarn.className = 'badge';
        el.thHumAlarm.textContent = '--';
        el.thHumAlarm.className = 'badge';
        el.thStatus.textContent = 'Menunggu sinkron';
        el.thStatus.className = 'badge badge-muted';
    }

    el.alarmState.className = `badge ${badgeClass('alarm', latest.alarmState)}`;
    el.alarmState.textContent = latest.alarmState;

    el.fan1State.className = `badge ${badgeClass('fan', latest.fan1On)}`;
    el.fan1State.textContent = latest.fan1On ? 'ON' : 'OFF';

    el.fan2State.className = `badge ${badgeClass('fan', latest.fan2On)}`;
    el.fan2State.textContent = latest.fan2On ? 'ON' : 'OFF';

    el.doorState.className = `badge ${badgeClass('door', latest.doorState)}`;
    el.doorState.textContent = latest.doorState;

    updateAlertState(latest);

    const now = Date.now();
    // Hitung ringkasan akses 24 jam terakhir.
    const dayAgo = now - 86400000;
    const access24 = access.filter(x => x.timestamp && x.timestamp.getTime() >= dayAgo);
    const granted = access24.filter(x => x.result === 'GRANTED').length;
    const denied = access24.filter(x => x.result === 'DENIED').length;
    const lockout = access24.filter(x => x.result === 'LOCKOUT').length;

    el.grantedCount.textContent = granted;
    el.deniedCount.textContent = denied;
    el.lockoutCount.textContent = lockout;

    if (access.length > 0) {
        // Kalau ada data akses terbaru, bisa tampilkan peringatan cepat.
        const latestAccess = access[access.length - 1];
        const oneMinuteAgo = now - 60000;
        if (latestAccess.timestamp.getTime() >= oneMinuteAgo) {
            if (latestAccess.result === 'DENIED') {
                showToast('Akses Ditolak', `Percobaan gagal oleh ${latestAccess.userId || 'Tidak Dikenal'} — ${latestAccess.reason}`, 'warning');
            } else if (latestAccess.result === 'LOCKOUT') {
                showToast('Sistem Terkunci', `Terminal terkunci — ${latestAccess.reason}`, 'danger');
            }
        }
    }

    const trend = telemetry.slice(-60);
    trendChart.data.labels = trend.map(x =>
        x.timestamp.toLocaleTimeString('id-ID', { hour: '2-digit', minute: '2-digit' })
    );
    trendChart.data.datasets[0].data = trend.map(x => x.temperature);
    trendChart.data.datasets[1].data = trend.map(x => x.humidity);
    trendChart.update('none');
}

async function refresh() {
    // Satu kali muat ulang mengambil data pantauan dan akses sekaligus.
    // Ambil ID spreadsheet dari browser.
    // Ini membuat dashboard bisa dipakai tanpa setting ulang setiap saat.
    const sheetId = loadConfig();
    if (!sheetId) {
        setStatus(false, 'Spreadsheet belum dikonfigurasi');
        return;
    }

    try {
        const [telemetryRaw, accessRaw] = await Promise.all([
            fetchSheet(sheetId, 'telemetry_logs'),
            fetchSheet(sheetId, 'access_logs')
        ]);

        telemetryData = parseTelemetry(telemetryRaw);
        accessData = parseAccess(accessRaw);

        if (telemetryData.length === 0) {
            // Kalau belum ada data, beri status yang jelas.
            setStatus(false, 'Tidak ada data');
            return;
        }

        // Gambar ulang semua bagian halaman.
        updateSummary(telemetryData, accessData);
        renderTelemetryTable(telemetryData, telemetryPage);
        renderAccessTable(accessData, accessPage);
        setStatus(true, 'Terhubung');
        el.lastUpdate.textContent = 'Diperbarui: ' + new Date().toLocaleTimeString('id-ID');
    } catch (err) {
        console.error(err);
        setStatus(false, 'Kesalahan koneksi');
    }
}

el.saveBtn.addEventListener('click', () => {
    // Simpan ID spreadsheet lalu muat ulang data.
    const id = saveConfig();
    if (id) refresh();
});

el.notifToggle.addEventListener('change', () => {
    // Simpan pilihan notifikasi saat saklar diubah.
    localStorage.setItem(NOTIF_KEY, el.notifToggle.checked);
});

if (el.refreshBtn) el.refreshBtn.addEventListener('click', refresh);

// Jalankan saat halaman dibuka.
initChart();
loadConfig();
refresh();
// Refresh otomatis setiap beberapa detik.
setInterval(refresh, REFRESH_INTERVAL);
