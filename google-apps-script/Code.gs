/**
 * Logger Smart Server.
 *
 * Catatan penting:
 * - harus ada pilihan lembar data
 * - hanya ada dua lembar data yang dipakai
 * - nama kolom harus konsisten
 * - tidak ada jalur cadangan
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
  "warn_threshold",
  "stage2_threshold",
  "warn_hum_threshold",
  "stage2_hum_threshold",
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
  // Jalur GET diperlakukan sama dengan jalur lain.
  return handleRequest(e);
}

function doPost(e) {
  // Jalur POST juga masuk ke handler yang sama.
  return handleRequest(e);
}

function handleRequest(e) {
  try {
    // Ambil semua parameter yang dikirim dari luar.
    // Setelah itu, data dipilih sesuai lembar yang diminta.
    const params = readParams(e);
    // Cari lembar data yang diminta.
    const sheetName = requireSheet(params);
    // Ambil spreadsheet aktif.
    const ss = SpreadsheetApp.getActiveSpreadsheet();
    // Pastikan lembar data ada.
    const sheet = ensureSheet(ss, sheetName);
    // Pastikan judul kolom sesuai.
    ensureHeaders(sheet, sheetName);

    // Susun satu baris data sesuai jenis lembar.
    const row =
      sheetName === ACCESS_SHEET
        ? buildAccessRow(params)
        : buildTelemetryRow(params);

    // Tambahkan baris data ke spreadsheet.
    sheet.appendRow(row);
    // Beri jawaban sukses ke pengirim.
    return jsonOutput({
      ok: true,
      sheet: sheetName,
      appendedAt: new Date().toISOString(),
    });
  } catch (err) {
    // Kalau ada masalah, kembalikan pesan gagal.
    return jsonOutput({
      ok: false,
      error: String(err),
    });
  }
}

function readParams(e) {
  // Baca parameter dari query string dan body JSON kalau ada.
  // Jadi kiriman dari browser maupun ESP32 bisa dipakai.
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
  // Hanya dua lembar data yang diizinkan.
  const sheet = String(params.sheet || "").trim();
  if (sheet !== TELEMETRY_SHEET && sheet !== ACCESS_SHEET) {
    throw new Error("invalid or missing sheet");
  }
  return sheet;
}

function ensureSheet(ss, sheetName) {
  // Cari lembar yang sudah ada.
  let sheet = ss.getSheetByName(sheetName);
  // Kalau belum ada, buat baru.
  if (!sheet) sheet = ss.insertSheet(sheetName);
  return sheet;
}

function ensureHeaders(sheet, sheetName) {
  // Kalau judul kolom berubah atau belum ada, tulis ulang supaya rapi.
  // Ini menjaga sheet tetap cocok dengan urutan data yang dikirim.
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
  // Susun satu baris data pantauan sesuai urutan judul kolom.
  // Urutan ini harus sama dengan header sheet.
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
    toNumber(params.warn_threshold, "warn_threshold"),
    toNumber(params.stage2_threshold, "stage2_threshold"),
    toNumber(params.warn_hum_threshold, "warn_hum_threshold"),
    toNumber(params.stage2_hum_threshold, "stage2_hum_threshold"),
  ];
}

function buildAccessRow(params) {
  // Susun satu baris data akses sesuai urutan header sheet.
  // Data ini dipakai untuk melihat siapa masuk atau ditolak.
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
  // Timestamp boleh kosong, angka Unix, atau string tanggal.
  // Bentuk apa pun akan diubah ke format tanggal biasa.
  if (value === undefined || value === null || String(value).trim() === "") {
    // Kalau kosong, pakai waktu sekarang.
    return new Date();
  }
  if (/^\d+$/.test(String(value))) {
    const n = Number(value);
    if (!isNaN(n) && n > 0) {
      // Kalau bentuknya angka, ubah ke tanggal yang sesuai.
      if (String(value).length <= 10) return new Date(n * 1000);
      return new Date(n);
    }
  }
  const dt = new Date(value);
  if (!isNaN(dt.getTime())) return dt;
  // Kalau tidak bisa dibaca, tolak.
  throw new Error("invalid timestamp");
}

function requireString(value, fieldName) {
  // Kolom teks wajib diisi agar catatan tidak kosong.
  // Kalau kosong, kiriman dianggap salah.
  if (value === undefined || value === null || String(value).trim() === "") {
    throw new Error("missing field: " + fieldName);
  }
  return String(value);
}

function toNumber(value, fieldName) {
  // Field angka wajib valid.
  // Kalau bukan angka, data ditolak supaya sheet tetap bersih.
  if (value === undefined || value === null || String(value).trim() === "") {
    throw new Error("missing field: " + fieldName);
  }
  const n = Number(value);
  if (isNaN(n)) throw new Error("invalid number field: " + fieldName);
  return n;
}

function toBooleanText(value) {
  // Boolean disimpan sebagai teks "true" / "false" supaya konsisten di sheet.
  // Dengan begitu, data mudah dibaca lagi dari web.
  const raw = String(value || "").toLowerCase();
  if (raw === "true" || raw === "1" || raw === "on") return "true";
  if (raw === "false" || raw === "0" || raw === "off") return "false";
  throw new Error("invalid boolean field");
}

function jsonOutput(obj) {
  // Ubah jawaban jadi JSON supaya mudah dibaca pengirim.
  return ContentService.createTextOutput(JSON.stringify(obj)).setMimeType(
    ContentService.MimeType.JSON
  );
}
