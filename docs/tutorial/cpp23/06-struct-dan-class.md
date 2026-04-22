# 06 - Struct dan Class

Bab ini menjelaskan cara membuat bentuk data sendiri.

## `struct`

`struct` dipakai untuk mengelompokkan data.

Kalau dibayangkan, `struct` itu seperti amplop yang isinya beberapa data terkait.

```cpp
// Wadah sederhana untuk satu hasil baca sensor.
struct Sensor {
    // Suhu yang dibaca sensor.
    int suhu;
    // Kelembapan yang dibaca sensor.
    int kelembapan;
};
```

## `class`

`class` mirip `struct`, tapi biasanya dipakai untuk menyembunyikan bagian dalam.

`class` cocok kalau kita ingin data dan tindakannya disatukan jadi satu benda.

```cpp
class Kipas {
public:
    // Simpan pin yang dipakai kipas.
    explicit Kipas(int pin) : pin_(pin) {}

    void mulai() {
        // Siapkan pin untuk keluaran.
        pinMode(pin_, OUTPUT);
        // Pastikan kipas mati dulu saat mulai.
        matikan();
    }

    void nyalakan() {
        // Kirim sinyal nyala ke relay.
        digitalWrite(pin_, HIGH);
        // Simpan status di dalam benda ini.
        aktif_ = true;
    }

    void matikan() {
        // Kirim sinyal mati ke relay.
        digitalWrite(pin_, LOW);
        // Simpan status di dalam benda ini.
        aktif_ = false;
    }

    // Beri tahu apakah kipas sedang aktif.
    bool status() const {
        return aktif_;
    }

private:
    int pin_;
    bool aktif_ = false;
};
```

## Bedanya Singkat

- `struct` biasanya dipakai untuk data sederhana
- `class` biasanya dipakai untuk data dan perilaku yang lebih lengkap

## Constructor

Constructor adalah fungsi khusus yang dipanggil saat benda baru dibuat.

```cpp
class Lampu {
public:
    // Simpan pin sejak benda dibuat.
    Lampu(int pin) : pin_(pin) {}
    // Nyalakan lampu lewat pin.
    void nyalakan() { digitalWrite(pin_, HIGH); }

private:
    int pin_;
};
```

## Saran

Kalau belum terbiasa, pikirkan `struct` sebagai kotak data dan `class` sebagai benda yang punya data dan aksi.
