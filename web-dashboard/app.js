const CONFIG_KEY = 'smart-server-sheet-id';
const DEFAULT_SHEET_ID = '1OjxwAK8oesknw5RHOvnRcrh6mAZCzVf5kH0vnfPLq4E';
const REFRESH_INTERVAL = 60000;
const NOTIF_KEY = 'smart-server-notif';
const PAGE_SIZE = 10;

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
    thStatus: document.getElementById('th-status')
};

let trendChart = null;
let telemetryData = [];
let accessData = [];
let telemetryPage = 1;
let accessPage = 1;

function initChart() {
    const ctx = document.getElementById('trend-chart').getContext('2d');

    const tempGrad = ctx.createLinearGradient(0, 0, 0, 320);
    tempGrad.addColorStop(0, 'rgba(79, 143, 255, 0.3)');
    tempGrad.addColorStop(1, 'rgba(79, 143, 255, 0.0)');

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
    const savedSheet = localStorage.getItem(CONFIG_KEY);
    el.sheetId.value = savedSheet || DEFAULT_SHEET_ID;
    el.notifToggle.checked = localStorage.getItem(NOTIF_KEY) === 'true';
    return el.sheetId.value;
}

function saveConfig() {
    const value = (el.sheetId.value || '').trim();
    if (!value) return null;
    localStorage.setItem(CONFIG_KEY, value);
    localStorage.setItem(NOTIF_KEY, el.notifToggle.checked);
    showToast('Terhubung', 'ID Spreadsheet tersimpan', 'success');
    return value;
}

function showToast(title, message, type = 'warning') {
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
    const url = `https://docs.google.com/spreadsheets/d/${sheetId}/gviz/tq?tqx=out:json&sheet=${encodeURIComponent(sheetName)}`;
    const response = await fetch(url);
    const text = await response.text();
    const jsonMatch = text.match(/google\.visualization\.Query\.setResponse\(([\s\S]*)\);?$/);
    if (!jsonMatch) throw new Error(`Respons tidak valid untuk ${sheetName}`);
    return JSON.parse(jsonMatch[1]);
}

function parseTelemetry(gviz) {
    return (gviz?.table?.rows || []).map(r => {
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
            stage2Threshold: c[10]?.v != null ? Number(c[10].v) : null
        };
    }).filter(x => x.timestamp);
}

function parseAccess(gviz) {
    return (gviz?.table?.rows || []).map(r => {
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
    el.statusDot.classList.toggle('online', online);
    el.statusDot.classList.toggle('offline', !online);
    el.statusText.textContent = text;
}

function badgeClass(type, value) {
    const map = {
        fan: value ? 'badge-on' : 'badge-off',
        alarm: value === 'ALARM' ? 'badge-alarm' : 'badge-normal',
        door: value === 'LOCKED' ? 'badge-locked' : 'badge-unlocked',
        result: { GRANTED: 'badge-granted', DENIED: 'badge-denied', LOCKOUT: 'badge-lockout' }[value] || ''
    };
    return map[type] || map;
}

function rssiQuality(rssi) {
    if (rssi >= -50) return { text: 'Sangat Baik', cls: 'badge-success' };
    if (rssi >= -60) return { text: 'Baik', cls: 'badge-success' };
    if (rssi >= -70) return { text: 'Cukup', cls: 'badge-warning' };
    return { text: 'Lemah', cls: 'badge-danger' };
}

function safeFixed(v) { return isFinite(v) ? v.toFixed(1) : '--'; }

function renderPagination(container, totalItems, currentPage, onPageChange) {
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

function updateSummary(telemetry, access) {
    if (telemetry.length === 0) {
        el.tempValue.textContent = '--';
        el.humValue.textContent = '--';
        el.rssiValue.textContent = '--';
        return;
    }

    const latest = telemetry[telemetry.length - 1];

    el.tempValue.textContent = safeFixed(latest.temperature);
    el.humValue.textContent = safeFixed(latest.humidity);

    if (latest.wifiRssi !== 0) {
        el.rssiValue.textContent = latest.wifiRssi;
        const rq = rssiQuality(latest.wifiRssi);
        el.rssiQuality.innerHTML = `<span class="badge ${rq.cls}">${rq.text}</span>`;
    } else {
        el.rssiValue.textContent = '--';
        el.rssiQuality.textContent = '';
    }

    if (latest.warnThreshold != null && isFinite(latest.warnThreshold)) {
        el.tempThreshold.textContent = `Peringatan: ${latest.warnThreshold}°C · Alarm: ${latest.stage2Threshold}°C`;
        el.humThreshold.textContent = `Ambang batas tersinkron dari perangkat`;
        el.thWarn.textContent = `${latest.warnThreshold}°C`;
        el.thWarn.className = 'badge badge-warning';
        el.thAlarm.textContent = `${latest.stage2Threshold}°C`;
        el.thAlarm.className = 'badge badge-danger';
        el.thStatus.textContent = 'Tersinkron';
        el.thStatus.className = 'badge badge-success';
    } else {
        el.tempThreshold.textContent = 'Ambang batas: menunggu sinkron ESP';
        el.humThreshold.textContent = '';
        el.thWarn.textContent = '--';
        el.thWarn.className = 'badge';
        el.thAlarm.textContent = '--';
        el.thAlarm.className = 'badge';
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

    if (latest.warnThreshold != null && latest.temperature > latest.warnThreshold) {
        showToast('Suhu Tinggi', `${latest.temperature}°C melebihi ambang peringatan (${latest.warnThreshold}°C)`, 'danger');
    }

    const now = Date.now();
    const dayAgo = now - 86400000;
    const access24 = access.filter(x => x.timestamp && x.timestamp.getTime() >= dayAgo);
    const granted = access24.filter(x => x.result === 'GRANTED').length;
    const denied = access24.filter(x => x.result === 'DENIED').length;
    const lockout = access24.filter(x => x.result === 'LOCKOUT').length;

    el.grantedCount.textContent = granted;
    el.deniedCount.textContent = denied;
    el.lockoutCount.textContent = lockout;

    if (access.length > 0) {
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
            setStatus(false, 'Tidak ada data');
            return;
        }

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
    const id = saveConfig();
    if (id) refresh();
});

el.notifToggle.addEventListener('change', () => {
    localStorage.setItem(NOTIF_KEY, el.notifToggle.checked);
});

if (el.refreshBtn) el.refreshBtn.addEventListener('click', refresh);

initChart();
loadConfig();
refresh();
setInterval(refresh, REFRESH_INTERVAL);
