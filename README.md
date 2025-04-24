# Multi Plug  

This example creates a Multi Plug device using the ESP
Matter data model.

See the [docs](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html) for more information about building and flashing the firmware.

## Flashing Factory Partition

```bash
esptool.py write_flash 0x10000 mfg_binaries/*.bin
```

1. Generate the Factory Partition image & related files
```bash
esp-matter-mfg-tool -cn "Plug" -v 0xFFF2 -p 0x8001 --pai \
    -k $MATTER_SDK_PATH/credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Key.pem \
    -c $MATTER_SDK_PATH/credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Cert.pem \
    -cd $MATTER_SDK_PATH/credentials/test/certification-declaration/Chip-Test-CD-FFF2-8001.der \
    --vendor-name "Espressif" --product-name "Multi Plug" \
    --hw-ver 1 --hw-ver-str DevKit \
    --fixed-labels "0/fixedname/Multi Plug" "1/fixedname/Plug 1" "2/fixedname/Plug 2" "3/fixedname/Plug 3" "4/fixedname/Plug 4" \
    --passcode 89674523 --discriminator 2245
```

## QR Code

![QR Code](./mfg_binaries/b7bd2e21-89b0-4ddb-b05e-f6c8795a7805-qrcode.png)