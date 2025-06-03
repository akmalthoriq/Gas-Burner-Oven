<h1>GAS BURNER Oven Otomatis Untuk Proses Pengeringan Setelah Proses Powder Coating</h1>

Proyek ini adalah untuk tahapan pengeringan part-part setelah melalui proses powder coating.Alat ini merupakan sistem kontrol gas dan pemantik otomatis berdasarkan pembacaan suhu yang sudah disetting dan akan menjaga suhu tersebut selama beberapa menit sesuai dengan waktu yang sudah ditentukan.



<h1>PIN Input, Output, dan Fungsi</h1>
PIN INPUT
- PA1 pbStart: Berfungsi untuk menyalakan/mengaktifkan sistem
- PA2 pb Stop: Berfungsi untuk menghentikan sistem
- PB12 tombolMenu: Berfungsi untuk masuk ke mode menu, digunakan untuk mengatur parameter
- PB13 tombolPlus: Berfungsi untuk menambah value/nilai parameter yang ingin diatur
- PB14 tombolMin: Berfungsi untuk mengurangi value/nilai parameter yang ingin diatur
- PB15 tombolSet: Berfungsi untuk menyimpan parameter ke dalam EEPROM agar parameter yang sudah di setting tidak hilang

PIN OUTPUT
- PB11 pin SSR pemantik: Digunakan untuk mengatur nyala/mati pemantik
- PB10 pin SSR selenoid Gas: Digunakan untuk mengatur nyala/mati selenoid gas

PIN I2C
- PB6 Pin SCL i2c: Digunakan untuk menampilkan informasi dan pengaturan pada LCD 16x2
- PB7 Pin SDA i2c: Digunakan untuk menampilkan informasi dan pengaturan pada LCD 16x2.

PIN MAX6675
- PB5 MAXSO: Digunakan untuk pin SO dari sensor suhu MAX6675
- PB3 MAXSCK: Digunakan untuk pin SCK dari sensor suhu MAX6675
- PB4 MAXCS: Digunakan untuk pin CS dari sensor suhu MAX6675

<h1>Fitur Utama</h1>
- Pengaturan Timer: Menjaga agar tetap mempertahankan suhu target sampai timer habis/selesai
- Pengaturan Suhu Target: Menghentikan Gas dan Pemantik apabila sudah mencapai suhu target
- Pengaturan Suhu Minimum: Mengaktifkan Gas dan Pemantik apabila suhu mencapai suhu minimum yang diatur agar suhu tetap terjaga
- Penyimpanan EEPROM: Pengaturan timer dan suhu yang telah diatur dan sudah disimpan akan tetap tersimpan meskipun daya dimatikan
- Tampilan LCD Interaktif: Menampilkan Suhu dari pembacaan sensor,Menampilkan waktu hitung mundur saat sistem aktif, serta nilai 
  pengaturan yang telah diatur dalam mode menu dan nilai yang tersimpan

<h1>Cara Menggunakan Alat</h1>
1. Pastikan Steker power sudah tersambung ke sumber listrik
2. Nyalakan Alat dengan memutar selector ke posisi ON
3. Atur Parameter yang tersedia seperti Suhu Target,Suhu Minimum, dan Waktu durasi pada mode menu
4. Jika parameter sudah di setting sesuai keinginan maka simpan pengaturan dengan cara menekan tombol sett/simpan
5. Jika sudah tekan tombol hijau untuk memulai sistem,sistem akan berjalan ditandai dengan lampu indikator yang menyala warna merah
6. Setelah tombol hijau ditekan maka alat akan bekerja dengan menyalakan gas dan pemantik hingga suhu target tercapai, jika sudah tercapai    maka timer akan mulai menghitung mundur serta gas dan pemantik akan mati. jika saat suhu turun sudah mencapai suhu minimum, maka           pemantik dan gas akan menyala kembali hingga tercapai suhu target lagi. Kurang lebih seperti itu siklusnya hingga sistem selesai           ditandai dengan Lampu indikator berwarna hijau menyala.
7. Jika ingin menghentikan sistem maka tekan tombol berwarna hitam agar sistem berhenti.
8. Jika saat ditekan tombol hitam sistem tetap tidak berhenti, maka dapat menekan tombol emergency yang berwarna merah agar seluruh alat      mati demi keamanan.
