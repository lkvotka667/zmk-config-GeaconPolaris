import pathlib
import os
import unittest
import yaml

ROOT = pathlib.Path(__file__).resolve().parents[1]


class Led4Tests(unittest.TestCase):
    @unittest.skipUnless(os.environ.get('LED4_BUILD_ROOT'), 'build tree not supplied')
    def test_generated_dts_and_firmware(self):
        build = pathlib.Path(os.environ['LED4_BUILD_ROOT'])
        rows = yaml.safe_load((ROOT / 'build.yaml').read_text())['include']
        for row in rows:
            if 'led4' not in row.get('snippet', '').split():
                continue
            with self.subTest(target=row['artifact-name']):
                zephyr = build / row['artifact-name'] / 'zephyr'
                dts = (zephyr / 'zephyr.dts').read_text()
                conf = (zephyr / '.config').read_text()
                self.assertGreater((zephyr / 'zmk.uf2').stat().st_size, 0)
                self.assertNotIn('zephyr,display =', dts)
                self.assertIn('control-gpios = < &gpio1 0xf 0x1 >', dts)
                self.assertIn('psels = < 0x500002e >', dts)
                self.assertIn('chain-length = < 0x4 >', dts)
                self.assertRegex(dts, r'i2c1:[^{]+\{[^}]+status = "disabled"')
                self.assertIn('CONFIG_ZMK_ANIMATION=y', conf)
                self.assertIn('CONFIG_ZMK_DRIVER_EXT_POWER_TRANSIENT=y', conf)
                self.assertNotIn('CONFIG_ZMK_DISPLAY=y', conf)

    def test_matrix_preserves_oled_and_separates_artifacts(self):
        rows = yaml.safe_load((ROOT / 'build.yaml').read_text())['include']
        leds = [r for r in rows if 'led4' in r.get('snippet', '').split()]
        self.assertEqual(len(leds), 7)
        names = [r['artifact-name'] for r in rows if 'artifact-name' in r]
        self.assertEqual(len(names), len(set(names)))
        for row in leds:
            self.assertNotIn('nice_oled', row['shield'])
            self.assertNotIn('TPD_L', row['snippet'])
            self.assertTrue(row['artifact-name'].endswith('_LED4'))
        oled = [r for r in rows if 'nice_oled' in r['shield']]
        self.assertEqual(len(oled), 5)

    def test_wiring_and_power_owner(self):
        overlay = (ROOT / 'snippets/led4/led4.overlay').read_text()
        self.assertIn('NRF_PSEL(SPIM_MOSI, 1, 14)', overlay)
        self.assertIn('control-gpios = <&gpio1 15 GPIO_ACTIVE_LOW>', overlay)
        self.assertIn('chain-length = <4>', overlay)
        self.assertIn('ext-power = <&led_power>', overlay)
        self.assertIn('/delete-property/ zephyr,display', overlay)
        self.assertIn('#ifdef POLARIS_TPD_L', overlay)
        tpd = (ROOT / 'snippets/TPD_L/TPD_L.overlay').read_text()
        self.assertIn('#ifdef POLARIS_LED4', tpd)


if __name__ == '__main__':
    unittest.main()
