const CONFIG_KEY = 'smart-server-sheet-id';
const DEFAULT_SHEET_ID = '1OjxwAK8oesknw5RHOvnRcrh6mAZCzVf5kH0vnfPLq4E';
const REFRESH_INTERVAL = 60000;

const el = {
    statusDot: document.getElementById('status-dot'),
    statusText: document.getElementById('status-text'),
    tempValue: document.getElementById('temp-value'),
    humValue: document.getElementById('hum-value'),
    alarmState: document.getElementById('alarm-state'),
    fanState: document.getElementById('fan-state'),
    doorState: document.getElementById('door-state'),
    grantedCount: document.getElementById('granted-count'),
    deniedCount: document.getElementById('denied-count'),
    lockoutCount: document.getElementById('lockout-count'),
    lastUpdate: document.getElementById('last-update'),
    telemetryBody: document.getElementById('telemetry-body'),
    accessBody: document.getElementById('access-body'),
    sheetId: document.getElementById('sheet-id'),
    saveBtn: document.getElementById('save-btn'),
    configSection: document.getElementById('config-section')
};

let trendChart = null;

function initChart() {
    const ctx = document.getElementById('trend-chart').getContext('2d');
    trendChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                {
                    label: 'Temperature (C)',
                    data: [],
                    borderColor: '#22d3ee',
                    backgroundColor: 'rgba(34,211,238,0.15)',
                    fill: true,
                    tension: 0.35,
                    pointRadius: 1.5
                },
                {
                    label: 'Humidity (%)',
                    data: [],
                    borderColor: '#3b82f6',
                    backgroundColor: 'rgba(59,130,246,0.12)',
                    fill: true,
                    tension: 0.35,
                    pointRadius: 1.5
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                x: { ticks: { color: '#94a3b8' }, grid: { color: 'rgba(148,163,184,.1)' } },
                y: { ticks: { color: '#94a3b8' }, grid: { color: 'rgba(148,163,184,.1)' } }
            },
            plugins: {
                legend: { labels: { color: '#cbd5e1' } }
            }
        }
    });
}

function loadSheetId() {
    const saved = localStorage.getItem(CONFIG_KEY);
    if (saved) {
        el.sheetId.value = saved;
        return saved;
    }
    el.sheetId.value = DEFAULT_SHEET_ID;
    return DEFAULT_SHEET_ID;
}

function saveSheetId() {
    const value = (el.sheetId.value || '').trim();
    if (!value) return null;
    localStorage.setItem(CONFIG_KEY, value);
    return value;
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
    if (!jsonMatch) throw new Error(`Invalid response for sheet ${sheetName}`);
    return JSON.parse(jsonMatch[1]);
}

function parseTelemetry(gviz) {
    const rows = gviz?.table?.rows || [];
    return rows.map(r => {
        const c = r.c || [];
        return {
            timestamp: parseGvizDate(c[0]?.v),
            deviceId: String(c[1]?.v || ''),
            temperature: Number(c[2]?.v || 0),
            humidity: Number(c[3]?.v || 0),
            fan1On: String(c[4]?.v || '').toLowerCase() === 'true',
            fan2On: String(c[5]?.v || '').toLowerCase() === 'true',
            alarmState: String(c[6]?.v || 'NORMAL'),
            doorState: String(c[7]?.v || 'LOCKED'),
            wifiRssi: Number(c[8]?.v || 0)
        };
    }).filter(x => x.timestamp && x.temperature > 0);
}

function parseAccess(gviz) {
    const rows = gviz?.table?.rows || [];
    return rows.map(r => {
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
        day: '2-digit',
        month: 'short',
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit'
    });
}

function setStatus(online, text) {
    el.statusDot.classList.toggle('online', online);
    el.statusDot.classList.toggle('offline', !online);
    el.statusText.textContent = text;
}

function renderTelemetryTable(items) {
    const latest50 = items.slice(-50).reverse();
    el.telemetryBody.innerHTML = latest50.map(x => `
      <tr>
        <td>${fmtTime(x.timestamp)}</td>
        <td>${x.temperature.toFixed(1)}</td>
        <td>${x.humidity.toFixed(1)}</td>
        <td>${x.fan1On ? 'ON' : 'OFF'}</td>
        <td>${x.fan2On ? 'ON' : 'OFF'}</td>
        <td>${x.alarmState}</td>
        <td>${x.doorState}</td>
      </tr>
    `).join('');
}

function renderAccessTable(items) {
    const latest50 = items.slice(-50).reverse();
    el.accessBody.innerHTML = latest50.map(x => `
      <tr>
        <td>${fmtTime(x.timestamp)}</td>
        <td>${x.userId || '-'}</td>
        <td>${x.displayName || '-'}</td>
        <td>${x.result || '-'}</td>
        <td>${x.reason || '-'}</td>
        <td>${x.failedCount}</td>
        <td>${x.doorState || '-'}</td>
      </tr>
    `).join('');
}

function updateSummary(telemetry, access) {
    if (telemetry.length === 0) return;
    const latest = telemetry[telemetry.length - 1];
    el.tempValue.textContent = latest.temperature.toFixed(1);
    el.humValue.textContent = latest.humidity.toFixed(1);
    el.alarmState.textContent = `Alarm: ${latest.alarmState}`;
    el.fanState.textContent = `Fans: F1 ${latest.fan1On ? 'ON' : 'OFF'} | F2 ${latest.fan2On ? 'ON' : 'OFF'}`;
    el.doorState.textContent = `Door: ${latest.doorState}`;

    const now = Date.now();
    const dayAgo = now - 24 * 60 * 60 * 1000;
    const access24 = access.filter(x => x.timestamp && x.timestamp.getTime() >= dayAgo);
    const granted = access24.filter(x => x.result === 'GRANTED').length;
    const denied = access24.filter(x => x.result === 'DENIED').length;
    const lockout = access24.filter(x => x.result === 'LOCKOUT').length;
    el.grantedCount.textContent = `Granted: ${granted}`;
    el.deniedCount.textContent = `Denied: ${denied}`;
    el.lockoutCount.textContent = `Lockout: ${lockout}`;

    const trend = telemetry.slice(-60);
    trendChart.data.labels = trend.map(x =>
        x.timestamp.toLocaleTimeString('id-ID', { hour: '2-digit', minute: '2-digit' })
    );
    trendChart.data.datasets[0].data = trend.map(x => x.temperature);
    trendChart.data.datasets[1].data = trend.map(x => x.humidity);
    trendChart.update('none');
}

async function refresh() {
    const sheetId = loadSheetId();
    if (!sheetId) {
        setStatus(false, 'Spreadsheet not configured');
        return;
    }

    try {
        const [telemetryRaw, accessRaw] = await Promise.all([
            fetchSheet(sheetId, 'telemetry_logs'),
            fetchSheet(sheetId, 'access_logs')
        ]);

        const telemetry = parseTelemetry(telemetryRaw);
        const access = parseAccess(accessRaw);

        if (telemetry.length === 0) {
            setStatus(false, 'No telemetry data');
            return;
        }

        updateSummary(telemetry, access);
        renderTelemetryTable(telemetry);
        renderAccessTable(access);
        setStatus(true, 'Connected');
        el.lastUpdate.textContent = 'Last update: ' + new Date().toLocaleTimeString('id-ID');
    } catch (err) {
        console.error(err);
        setStatus(false, 'Connection error');
    }
}

el.saveBtn.addEventListener('click', () => {
    const id = saveSheetId();
    if (id) refresh();
});

initChart();
loadSheetId();
refresh();
setInterval(refresh, REFRESH_INTERVAL);
