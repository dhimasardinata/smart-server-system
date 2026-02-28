/**
 * Smart Server Logger (Strict Mode)
 *
 * Rules:
 * - `sheet` is mandatory and must be:
 *   - telemetry_logs
 *   - access_logs
 * - Snake_case fields only.
 * - No legacy/fallback routing.
 */

const TELEMETRY_SHEET = "telemetry_logs";
const ACCESS_SHEET = "access_logs";

const TELEMETRY_HEADERS = [
  "timestamp",
  "device_id",
  "temperature_c",
  "humidity_pct",
  "fan1_on",
  "fan2_on",
  "alarm_state",
  "door_state",
  "wifi_rssi",
];

const ACCESS_HEADERS = [
  "timestamp",
  "device_id",
  "user_id",
  "display_name",
  "result",
  "reason",
  "failed_count",
  "lockout_until",
  "door_state",
];

function doGet(e) {
  return handleRequest(e);
}

function doPost(e) {
  return handleRequest(e);
}

function handleRequest(e) {
  try {
    const params = readParams(e);
    const sheetName = requireSheet(params);
    const ss = SpreadsheetApp.getActiveSpreadsheet();
    const sheet = ensureSheet(ss, sheetName);
    ensureHeaders(sheet, sheetName);

    const row =
      sheetName === ACCESS_SHEET
        ? buildAccessRow(params)
        : buildTelemetryRow(params);

    sheet.appendRow(row);
    return jsonOutput({
      ok: true,
      sheet: sheetName,
      appendedAt: new Date().toISOString(),
    });
  } catch (err) {
    return jsonOutput({
      ok: false,
      error: String(err),
    });
  }
}

function readParams(e) {
  const query = (e && e.parameter) || {};
  let body = {};

  if (e && e.postData && e.postData.contents) {
    const raw = e.postData.contents;
    if (raw && raw.trim().length > 0) {
      try {
        body = JSON.parse(raw);
      } catch (_) {
        // Ignore malformed JSON body and keep query values.
      }
    }
  }

  return Object.assign({}, query, body);
}

function requireSheet(params) {
  const sheet = String(params.sheet || "").trim();
  if (sheet !== TELEMETRY_SHEET && sheet !== ACCESS_SHEET) {
    throw new Error("invalid or missing sheet");
  }
  return sheet;
}

function ensureSheet(ss, sheetName) {
  let sheet = ss.getSheetByName(sheetName);
  if (!sheet) sheet = ss.insertSheet(sheetName);
  return sheet;
}

function ensureHeaders(sheet, sheetName) {
  const headers = sheetName === ACCESS_SHEET ? ACCESS_HEADERS : TELEMETRY_HEADERS;
  const range = sheet.getRange(1, 1, 1, headers.length);
  const current = range.getValues()[0];

  let rewrite = false;
  for (let i = 0; i < headers.length; i++) {
    if (String(current[i] || "") !== headers[i]) {
      rewrite = true;
      break;
    }
  }
  if (rewrite) range.setValues([headers]);
}

function buildTelemetryRow(params) {
  return [
    normalizeTimestamp(params.timestamp),
    requireString(params.device_id, "device_id"),
    toNumber(params.temperature_c, "temperature_c"),
    toNumber(params.humidity_pct, "humidity_pct"),
    toBooleanText(params.fan1_on),
    toBooleanText(params.fan2_on),
    requireString(params.alarm_state, "alarm_state"),
    requireString(params.door_state, "door_state"),
    toNumber(params.wifi_rssi, "wifi_rssi"),
  ];
}

function buildAccessRow(params) {
  return [
    normalizeTimestamp(params.timestamp),
    requireString(params.device_id, "device_id"),
    requireString(params.user_id, "user_id"),
    requireString(params.display_name, "display_name"),
    requireString(params.result, "result"),
    requireString(params.reason, "reason"),
    toNumber(params.failed_count, "failed_count"),
    toNumber(params.lockout_until, "lockout_until"),
    requireString(params.door_state, "door_state"),
  ];
}

function normalizeTimestamp(value) {
  if (value === undefined || value === null || String(value).trim() === "") {
    return new Date();
  }
  if (/^\d+$/.test(String(value))) {
    const n = Number(value);
    if (!isNaN(n) && n > 0) {
      if (String(value).length <= 10) return new Date(n * 1000);
      return new Date(n);
    }
  }
  const dt = new Date(value);
  if (!isNaN(dt.getTime())) return dt;
  throw new Error("invalid timestamp");
}

function requireString(value, fieldName) {
  if (value === undefined || value === null || String(value).trim() === "") {
    throw new Error("missing field: " + fieldName);
  }
  return String(value);
}

function toNumber(value, fieldName) {
  if (value === undefined || value === null || String(value).trim() === "") {
    throw new Error("missing field: " + fieldName);
  }
  const n = Number(value);
  if (isNaN(n)) throw new Error("invalid number field: " + fieldName);
  return n;
}

function toBooleanText(value) {
  const raw = String(value || "").toLowerCase();
  if (raw === "true" || raw === "1" || raw === "on") return "true";
  if (raw === "false" || raw === "0" || raw === "off") return "false";
  throw new Error("invalid boolean field");
}

function jsonOutput(obj) {
  return ContentService.createTextOutput(JSON.stringify(obj)).setMimeType(
    ContentService.MimeType.JSON
  );
}
