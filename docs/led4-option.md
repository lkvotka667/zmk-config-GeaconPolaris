# OLED交換用4連LEDモジュール

専用ブランチ: `feature/polaris-oled-led4`。通常のOLEDファームは変更しません。
`build.yaml` の末尾が `_LED4` のファームを選び、電源を切ってOLEDをLEDモジュールへ交換してください。
左LPPS/TB/JOY/ENC、右TB/TPD/IQSを用意しています。左右は個別に選択できます。

## 配線

| OLEDコネクター | LEDモジュール | XIAO |
| --- | --- | --- |
| SDA | LEDデータ（先頭DIN） | D9 / P1.14 |
| SCL | LED_EN（Lowで電源ON） | D10 / P1.15 |
| VCC / GND | 電源 / GND | 既存端子 |

提示回路図の接続を前提としています。LED型番・GRB順序・電源ゲート極性は実機確認が必要です。
左TPDはDR信号にP1.14を使用するため併用不可です（両snippetの指定はビルドエラー）。
LED用SPI3はデータのみ使用し、TBのSPI0とは分離します。I2C1と表示機能はLED版で無効化します。

## 表示と消灯

Equinoxと同じanimation / ext-power-transientモジュールを固定SHAで使用します。
起動時は電池状態→消灯→接続状態、USB給電時は接続表示（虹色へ切替可能）、
電池駆動時は通常消灯し、復帰時に電池状態を短時間表示します。idle時はアニメーション停止。
電源ゲートはanimation側だけで管理します。XIAO内蔵RGBは従来どおりです。

既存キー配置は変更しません。必要に応じてキーへ以下を割り当てられます。

```dts
&animctl ANM_DS   // アニメーション無効・LED電源OFF
&animctl ANM_EN   // 再度有効化
&animctl ANM_INC  // 表示モード切替
```

これらは `led4` ビルド専用です。通常OLED版の共通keymapへ無条件に追加しないでください。
未検証: 実機点灯、電源OFF時の消費電流、スリープ復帰、各入力モジュールとの同時動作。

## ビルド検証（2026-10-22）

- LED4版7ターゲットのUF2生成成功。
- 通常OLED版 `Polaris_L_MODULE_ENC` の回帰ビルド成功。
- 生成DTS／設定の検査を含む3テスト成功。7ターゲットについてOLED無効化、
  P1.14 MOSI、P1.15 active-low、4LED、animation／電源制御有効化を確認。
- 実機への書き込みは未実施。上記は実機検証の代替ではありません。
