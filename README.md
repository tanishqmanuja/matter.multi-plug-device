# Multi Plug  

This example creates a Multi Plug device using the ESP
Matter data model.

See the [docs](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html) for more information about building and flashing the firmware.

## Flashing Factory Partition

```bash
esptool.py write_flash 0x10000 mfg_binaries/*.bin
```

## Generating Factory Partition

1. Genrate the Certification Declaration
```bash
out/host/chip-cert gen-cd -f 1 -V 0xFFF1 -p 0x8001 -d 0x0016 \
                          -c "CSA00000SWC00000-01" -l 0 -i 0 -n 1 -t 0 \
                          -K credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem \
                          -C credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem \
                          -O TEST_CD_FFF1_8001.der
```

2. Generate the Factory Partition image & related files
```bash
./mfg_tool.py --passcode 89674523 \
              --discriminator 2245 \
              -cd TEST_CD_FFF1_8001.der \
              -cn Plug \
              -v 0xFFF1 --vendor-name Espressif \
              -p 0x8001 --product-name "Multi Plug" \
              --hw-ver 1 --hw-ver-str DevKit \
              --fixed-labels "0/fixedname/Multi Plug" "1/fixedname/Plug 1" "2/fixedname/Plug 2" "3/fixedname/Plug 3" "4/fixedname/Plug 4"
```

## QR Code

![QR Code](./mfg_binaries/f013439d-9ee1-4539-a7c1-18b0886e3fe4-qrcode.png)