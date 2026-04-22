# 08 - OOP

OOP berarti cara menulis program dengan benda-benda kecil yang punya data dan aksi.

## Contoh Cara Pikir

Daripada membuat kode besar, kita bisa membagi menjadi:

- sensor
- layar
- jaringan
- pengatur akses

## Kenapa Dipakai

Karena:

- lebih rapi
- lebih mudah dirawat
- lebih gampang dipisah tugasnya

## Contoh Sederhana

```cpp
class Kipas {
public:
    // Simpan pin untuk alat ini.
    explicit Kipas(int pin) : pin_(pin) {}

    void mulai() {
        // Siapkan pin sebagai keluaran.
        pinMode(pin_, OUTPUT);
        // Mulai dari keadaan mati.
        matikan();
    }

    void nyalakan() {
        // Aktifkan relay atau pin.
        digitalWrite(pin_, HIGH);
        // Simpan status aktif.
        aktif = true;
    }

    void matikan() {
        // Matikan relay atau pin.
        digitalWrite(pin_, LOW);
        // Simpan status mati.
        aktif = false;
    }

    // Baca status alat saat ini.
    bool status() const { return aktif; }

private:
    int pin_;
    bool aktif = false;
};
```

## `virtual`

Kalau satu benda punya beberapa versi perilaku, `virtual` dipakai supaya versi yang tepat dipanggil.

Contoh gampangnya: alat yang sama bisa punya cara kerja yang berbeda tergantung jenisnya.

Di ESP32, ini berguna kalau nanti kamu punya beberapa jenis sensor, tapi semuanya mau diperlakukan lewat satu pintu yang sama.

## Saran

Untuk belajar awal, fokus dulu pada:

- data apa yang disimpan
- aksi apa yang dilakukan
- bagian mana yang sebaiknya dipisah
