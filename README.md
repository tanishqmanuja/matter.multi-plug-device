# Multi Plug  

This example creates a Multi Plug device using the ESP Matter data model.

See the [docs](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html) for more information about building and flashing the firmware.

## Flashing Factory Partition

1. First Flash the Factory Partition
```bash
esptool.py write_flash 0x10000 mfg_binaries/*.bin
```

2. Flash the Firmware
```bash
idf.py flash
```

Generate the Factory Partition image & related files
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


### Multi Plug Device Configuration

| Config Option               | Type   | Default | Description                                |
|----------------------------|--------|---------|--------------------------------------------|
| `CONFIG_NUM_PLUGS`         | `int`  | 4       | Number of total plugs (range: 1–6)         |
| `CONFIG_ACTIVE_HIGH_PLUGS`| `bool` | `y`     | Logic level: `y` for active-high plugs     |

### Default GPIO Pin Mapping

| Plug Component | Config Option               | Default GPIO |
|----------------|-----------------------------|--------------|
| Power Outlet   | `CONFIG_GPIO_POWER_OUTLET`  | 4            |
| Plug 1         | `CONFIG_GPIO_PLUG_1`        | 32           |
| Plug 2         | `CONFIG_GPIO_PLUG_2`        | 33           |
| Plug 3         | `CONFIG_GPIO_PLUG_3`        | 27           |
| Plug 4         | `CONFIG_GPIO_PLUG_4`        | 26           |
| Plug 5         | `CONFIG_GPIO_PLUG_5`        | 13           |
| Plug 6         | `CONFIG_GPIO_PLUG_6`        | 14           |

