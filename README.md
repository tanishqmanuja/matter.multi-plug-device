# Multi Plug  

This example creates a Multi Plug device using the ESP
Matter data model.

See the [docs](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html) for more information about building and flashing the firmware.

## Flashing Factory Partition

```bash
esptool.py write_flash 0x10000 mfg_binaries/*.bin
```

The factory partition was generated using the following command,

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
