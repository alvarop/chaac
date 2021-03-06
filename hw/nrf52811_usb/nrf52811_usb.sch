EESchema Schematic File Version 4
EELAYER 29 0
EELAYER END
$Descr A3 16535 11693
encoding utf-8
Sheet 1 1
Title "NRF52811_USB"
Date "2019-05-24"
Rev "1.0"
Comp "@alvaroprieto"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L alvarop:MIC550X U1
U 1 1 5BA99744
P 2100 2800
F 0 "U1" H 2100 3337 60  0000 C CNN
F 1 "MIC550X" H 2100 3231 60  0000 C CNN
F 2 "alvarop:SOT-23(M5)" H 2100 2800 60  0001 C CNN
F 3 "" H 2100 2800 60  0000 C CNN
F 4 "576-4764-1-ND" H -3750 1250 50  0001 C CNN "DKPN"
F 5 "MIC5504-3.3YM5-TR" H -3750 1250 50  0001 C CNN "MPN"
	1    2100 2800
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR07
U 1 1 5BA99856
P 2800 2400
F 0 "#PWR07" H 2800 2250 50  0001 C CNN
F 1 "+3.3V" H 2815 2573 50  0000 C CNN
F 2 "" H 2800 2400 50  0001 C CNN
F 3 "" H 2800 2400 50  0001 C CNN
	1    2800 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2800 2550 2800 2400
NoConn ~ 2500 2650
$Comp
L power:GND #PWR05
U 1 1 5BA99C01
P 2100 3100
F 0 "#PWR05" H 2100 2850 50  0001 C CNN
F 1 "GND" H 2105 2927 50  0000 C CNN
F 2 "" H 2100 3100 50  0001 C CNN
F 3 "" H 2100 3100 50  0001 C CNN
	1    2100 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 3100 2100 3050
Wire Wire Line
	1650 2550 1700 2550
Wire Wire Line
	1650 2550 1650 2650
Wire Wire Line
	1650 2650 1700 2650
Wire Wire Line
	1650 2550 1450 2550
Connection ~ 1650 2550
$Comp
L Device:C_Small C1
U 1 1 5BA9A53F
P 1450 2750
F 0 "C1" H 1542 2796 50  0000 L CNN
F 1 "1uF" H 1542 2705 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 1450 2750 50  0001 C CNN
F 3 "~" H 1450 2750 50  0001 C CNN
F 4 "311-1438-1-ND" H -3750 1250 50  0001 C CNN "DKPN"
F 5 "CC0402KRX5R5BB105" H -3750 1250 50  0001 C CNN "MPN"
	1    1450 2750
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C5
U 1 1 5BA9A5CD
P 2650 2750
F 0 "C5" H 2742 2796 50  0000 L CNN
F 1 "1uF" H 2742 2705 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 2650 2750 50  0001 C CNN
F 3 "~" H 2650 2750 50  0001 C CNN
F 4 "311-1438-1-ND" H -3750 1250 50  0001 C CNN "DKPN"
F 5 "CC0402KRX5R5BB105" H -3750 1250 50  0001 C CNN "MPN"
	1    2650 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 2650 2650 2550
Connection ~ 2650 2550
Wire Wire Line
	2650 2550 2800 2550
Wire Wire Line
	1450 2650 1450 2550
Connection ~ 1450 2550
Wire Wire Line
	1450 3050 2100 3050
Wire Wire Line
	1450 2850 1450 3050
Connection ~ 2100 3050
Wire Wire Line
	2100 3050 2100 3000
Wire Wire Line
	2650 3050 2650 2850
NoConn ~ 14100 6700
Text Label 850  2550 2    50   ~ 0
VIN
$Comp
L power:+3.3V #PWR024
U 1 1 5C2CA526
P 8950 1850
F 0 "#PWR024" H 8950 1700 50  0001 C CNN
F 1 "+3.3V" H 8965 2023 50  0000 C CNN
F 2 "" H 8950 1850 50  0001 C CNN
F 3 "" H 8950 1850 50  0001 C CNN
	1    8950 1850
	1    0    0    -1  
$EndComp
$Comp
L alvarop:TC2030-CTX J3
U 1 1 5C2FAE4C
P 13550 6650
F 0 "J3" H 13550 7047 60  0000 C CNN
F 1 "TC2030-CTX" H 13550 6941 60  0000 C CNN
F 2 "alvarop:TC2030-NL" H 13550 6450 60  0001 C CNN
F 3 "" H 13550 6450 60  0001 C CNN
	1    13550 6650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR021
U 1 1 5C34AEF4
P 7350 2850
F 0 "#PWR021" H 7350 2600 50  0001 C CNN
F 1 "GND" H 7355 2677 50  0000 C CNN
F 2 "" H 7350 2850 50  0001 C CNN
F 3 "" H 7350 2850 50  0001 C CNN
	1    7350 2850
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR022
U 1 1 5C34B072
P 7350 3250
F 0 "#PWR022" H 7350 3000 50  0001 C CNN
F 1 "GND" H 7355 3077 50  0000 C CNN
F 2 "" H 7350 3250 50  0001 C CNN
F 3 "" H 7350 3250 50  0001 C CNN
	1    7350 3250
	0    1    1    0   
$EndComp
Wire Wire Line
	7350 3250 7400 3250
Wire Wire Line
	7350 2850 7400 2850
$Comp
L Switch:SW_Push SW2
U 1 1 5C367753
P 5450 3600
F 0 "SW2" H 5300 3700 50  0000 C CNN
F 1 "RST" H 5600 3700 50  0000 C CNN
F 2 "alvarop:PTS645SM43SMTR92" H 5450 3800 50  0001 C CNN
F 3 "" H 5450 3800 50  0001 C CNN
F 4 "CKN9112CT-ND" H 0   0   50  0001 C CNN "DKPN"
F 5 "PTS645SM43SMTR92 LFS" H 0   0   50  0001 C CNN "MPN"
	1    5450 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	5650 3600 5700 3600
Wire Wire Line
	5700 3600 5700 3650
$Comp
L power:GND #PWR017
U 1 1 5C3935DE
P 5700 3900
F 0 "#PWR017" H 5700 3650 50  0001 C CNN
F 1 "GND" H 5705 3727 50  0000 C CNN
F 2 "" H 5700 3900 50  0001 C CNN
F 3 "" H 5700 3900 50  0001 C CNN
	1    5700 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 3900 5700 3850
$Comp
L power:GND #PWR013
U 1 1 5C3A2335
P 5200 3650
F 0 "#PWR013" H 5200 3400 50  0001 C CNN
F 1 "GND" H 5205 3477 50  0000 C CNN
F 2 "" H 5200 3650 50  0001 C CNN
F 3 "" H 5200 3650 50  0001 C CNN
	1    5200 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	5200 3650 5200 3600
Wire Wire Line
	5200 3600 5250 3600
Wire Wire Line
	14100 6500 14400 6500
Wire Wire Line
	14100 6600 14400 6600
Wire Wire Line
	13000 6500 12900 6500
Wire Wire Line
	12900 6500 12900 6400
Wire Wire Line
	13000 6600 12750 6600
Wire Wire Line
	13000 6700 12900 6700
Wire Wire Line
	12900 6700 12900 6800
$Comp
L power:GND #PWR020
U 1 1 5C42D762
P 12900 6800
F 0 "#PWR020" H 12900 6550 50  0001 C CNN
F 1 "GND" H 12905 6627 50  0000 C CNN
F 2 "" H 12900 6800 50  0001 C CNN
F 3 "" H 12900 6800 50  0001 C CNN
	1    12900 6800
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR019
U 1 1 5C42D880
P 12900 6400
F 0 "#PWR019" H 12900 6250 50  0001 C CNN
F 1 "+3.3V" H 12915 6573 50  0000 C CNN
F 2 "" H 12900 6400 50  0001 C CNN
F 3 "" H 12900 6400 50  0001 C CNN
	1    12900 6400
	1    0    0    -1  
$EndComp
Text Label 12750 6600 0    50   ~ 0
nRST
Text Label 5850 3600 0    50   ~ 0
nRST
Text Label 14400 6500 2    50   ~ 0
SWDIO
Text Label 14400 6600 2    50   ~ 0
SWCLK
Wire Wire Line
	11000 3900 11400 3900
Wire Wire Line
	11000 4000 11400 4000
Text Label 11400 3900 2    50   ~ 0
SWDIO
Text Label 11400 4000 2    50   ~ 0
SWCLK
Wire Wire Line
	8700 3900 8150 3900
Wire Wire Line
	8700 3800 8150 3800
Text Label 8150 3800 0    50   ~ 0
UART_TX
Text Label 8150 3900 0    50   ~ 0
UART_RX
$Comp
L Device:C_Small C15
U 1 1 5C2FB65B
P 11400 2600
F 0 "C15" H 11250 2750 50  0000 L CNN
F 1 "0.1uF" H 11150 2500 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 11400 2600 50  0001 C CNN
F 3 "~" H 11400 2600 50  0001 C CNN
F 4 "490-1318-1-ND" H 11400 2600 50  0001 C CNN "DKPN"
F 5 "GRM155R61A104KA01D" H 11400 2600 50  0001 C CNN "MPN"
	1    11400 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 2550 2650 2550
$Comp
L Device:R_Small R3
U 1 1 5D68310B
P 5950 4500
F 0 "R3" V 6050 4550 50  0000 R CNN
F 1 "10k" V 5850 4550 50  0000 R CNN
F 2 "Resistor_SMD:R_0402_1005Metric" H 5950 4500 50  0001 C CNN
F 3 "~" H 5950 4500 50  0001 C CNN
F 4 "311-10KJRCT-ND" H 3700 -5700 50  0001 C CNN "DKPN"
F 5 "RC0402JR-0710KL" H 3700 -5700 50  0001 C CNN "MPN"
	1    5950 4500
	0    -1   -1   0   
$EndComp
$Comp
L Device:LED_ALT D1
U 1 1 5D683111
P 5550 4500
F 0 "D1" H 5541 4716 50  0000 C CNN
F 1 "LED_ALT" H 5541 4625 50  0000 C CNN
F 2 "LED_SMD:LED_0402_1005Metric" H 5550 4500 50  0001 C CNN
F 3 "~" H 5550 4500 50  0001 C CNN
F 4 "1516-1216-1-ND" H 0   0   50  0001 C CNN "DKPN"
F 5 "QBLP595-IG" H 0   0   50  0001 C CNN "MPN"
	1    5550 4500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 4500 5850 4500
Wire Wire Line
	6050 4500 6400 4500
Wire Wire Line
	5400 4500 5200 4500
$Comp
L power:GND #PWR014
U 1 1 5D6FB3B3
P 5200 4550
F 0 "#PWR014" H 5200 4300 50  0001 C CNN
F 1 "GND" H 5205 4377 50  0000 C CNN
F 2 "" H 5200 4550 50  0001 C CNN
F 3 "" H 5200 4550 50  0001 C CNN
	1    5200 4550
	1    0    0    -1  
$EndComp
Text Label 6400 4500 2    50   ~ 0
TX_LED
Wire Wire Line
	5200 4500 5200 4550
Wire Wire Line
	850  2550 1450 2550
Wire Wire Line
	8700 4000 8150 4000
Text Label 8150 4000 0    50   ~ 0
BOOT
$Comp
L Switch:SW_Push SW1
U 1 1 5C55D249
P 5400 2050
F 0 "SW1" H 5250 2150 50  0000 C CNN
F 1 "RST" H 5550 2150 50  0000 C CNN
F 2 "alvarop:PTS645SM43SMTR92" H 5400 2250 50  0001 C CNN
F 3 "" H 5400 2250 50  0001 C CNN
F 4 "CKN9112CT-ND" H -50 -1550 50  0001 C CNN "DKPN"
F 5 "PTS645SM43SMTR92 LFS" H -50 -1550 50  0001 C CNN "MPN"
	1    5400 2050
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR012
U 1 1 5C55D24F
P 5150 2100
F 0 "#PWR012" H 5150 1850 50  0001 C CNN
F 1 "GND" H 5155 1927 50  0000 C CNN
F 2 "" H 5150 2100 50  0001 C CNN
F 3 "" H 5150 2100 50  0001 C CNN
	1    5150 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 2100 5150 2050
Wire Wire Line
	5150 2050 5200 2050
Wire Wire Line
	5600 2050 6000 2050
Text Label 6000 2050 2    50   ~ 0
BOOT
$Comp
L Device:R_Small R4
U 1 1 5C5E3734
P 5950 5000
F 0 "R4" V 6050 5050 50  0000 R CNN
F 1 "10k" V 5850 5050 50  0000 R CNN
F 2 "Resistor_SMD:R_0402_1005Metric" H 5950 5000 50  0001 C CNN
F 3 "~" H 5950 5000 50  0001 C CNN
F 4 "311-10KJRCT-ND" H 3700 -5200 50  0001 C CNN "DKPN"
F 5 "RC0402JR-0710KL" H 3700 -5200 50  0001 C CNN "MPN"
	1    5950 5000
	0    -1   -1   0   
$EndComp
$Comp
L Device:LED_ALT D2
U 1 1 5C5E373C
P 5550 5000
F 0 "D2" H 5541 5216 50  0000 C CNN
F 1 "LED_ALT" H 5541 5125 50  0000 C CNN
F 2 "LED_SMD:LED_0402_1005Metric" H 5550 5000 50  0001 C CNN
F 3 "~" H 5550 5000 50  0001 C CNN
F 4 "1516-1216-1-ND" H 0   500 50  0001 C CNN "DKPN"
F 5 "QBLP595-IG" H 0   500 50  0001 C CNN "MPN"
	1    5550 5000
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 5000 5850 5000
Wire Wire Line
	6050 5000 6400 5000
Wire Wire Line
	5400 5000 5200 5000
$Comp
L power:GND #PWR015
U 1 1 5C5E3745
P 5200 5050
F 0 "#PWR015" H 5200 4800 50  0001 C CNN
F 1 "GND" H 5205 4877 50  0000 C CNN
F 2 "" H 5200 5050 50  0001 C CNN
F 3 "" H 5200 5050 50  0001 C CNN
	1    5200 5050
	1    0    0    -1  
$EndComp
Text Label 6400 5000 2    50   ~ 0
RX_LED
Wire Wire Line
	5200 5000 5200 5050
$Comp
L power:+3.3V #PWR030
U 1 1 5D0D2BBA
P 11150 2350
F 0 "#PWR030" H 11150 2200 50  0001 C CNN
F 1 "+3.3V" H 11165 2523 50  0000 C CNN
F 2 "" H 11150 2350 50  0001 C CNN
F 3 "" H 11150 2350 50  0001 C CNN
	1    11150 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	11150 2350 11150 2400
Wire Wire Line
	11150 2900 11000 2900
$Comp
L power:GND #PWR029
U 1 1 5D0F4473
P 9700 1500
F 0 "#PWR029" H 9700 1250 50  0001 C CNN
F 1 "GND" H 9705 1327 50  0000 C CNN
F 2 "" H 9700 1500 50  0001 C CNN
F 3 "" H 9700 1500 50  0001 C CNN
	1    9700 1500
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR026
U 1 1 5D116462
P 8950 4700
F 0 "#PWR026" H 8950 4550 50  0001 C CNN
F 1 "+3.3V" H 8965 4873 50  0000 C CNN
F 2 "" H 8950 4700 50  0001 C CNN
F 3 "" H 8950 4700 50  0001 C CNN
	1    8950 4700
	1    0    0    -1  
$EndComp
Text Label 10400 5150 1    50   ~ 0
nRST
Wire Wire Line
	10400 5150 10400 4600
Wire Wire Line
	5700 3600 5850 3600
Connection ~ 5700 3600
Wire Wire Line
	11400 2500 11400 2400
Wire Wire Line
	11400 2400 11150 2400
Connection ~ 11150 2400
Wire Wire Line
	11150 2400 11150 2900
$Comp
L power:GND #PWR031
U 1 1 5D4838A3
P 11400 2750
F 0 "#PWR031" H 11400 2500 50  0001 C CNN
F 1 "GND" H 11405 2577 50  0000 C CNN
F 2 "" H 11400 2750 50  0001 C CNN
F 3 "" H 11400 2750 50  0001 C CNN
	1    11400 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	11400 2750 11400 2700
Wire Wire Line
	11900 3350 12100 3350
$Comp
L Device:C_Small C17
U 1 1 5D527C74
P 12200 3350
F 0 "C17" V 12150 3400 50  0000 L CNN
F 1 "DNP" V 12150 3150 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 12200 3350 50  0001 C CNN
F 3 "~" H 12200 3350 50  0001 C CNN
	1    12200 3350
	0    1    1    0   
$EndComp
$Comp
L Device:C_Small C16
U 1 1 5D4A59D5
P 12200 3200
F 0 "C16" V 12150 3250 50  0000 L CNN
F 1 "100pF" V 12150 2950 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 12200 3200 50  0001 C CNN
F 3 "~" H 12200 3200 50  0001 C CNN
F 4 "587-1211-1-ND" H 12200 3200 50  0001 C CNN "DKPN"
F 5 " UMK105CG101JV-F" H 12200 3200 50  0001 C CNN "MPN"
	1    12200 3200
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR033
U 1 1 5D58C3B5
P 12400 3200
F 0 "#PWR033" H 12400 2950 50  0001 C CNN
F 1 "GND" H 12405 3027 50  0000 C CNN
F 2 "" H 12400 3200 50  0001 C CNN
F 3 "" H 12400 3200 50  0001 C CNN
	1    12400 3200
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR034
U 1 1 5D58C823
P 12400 3350
F 0 "#PWR034" H 12400 3100 50  0001 C CNN
F 1 "GND" H 12405 3177 50  0000 C CNN
F 2 "" H 12400 3350 50  0001 C CNN
F 3 "" H 12400 3350 50  0001 C CNN
	1    12400 3350
	0    -1   -1   0   
$EndComp
Wire Wire Line
	12300 3350 12400 3350
Wire Wire Line
	12300 3200 12400 3200
$Comp
L Device:C_Small C13
U 1 1 5D5CDBB6
P 8950 4900
F 0 "C13" H 8800 5050 50  0000 L CNN
F 1 "0.1uF" H 8700 4800 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 8950 4900 50  0001 C CNN
F 3 "~" H 8950 4900 50  0001 C CNN
F 4 "490-1318-1-ND" H 8950 4900 50  0001 C CNN "DKPN"
F 5 "GRM155R61A104KA01D" H 8950 4900 50  0001 C CNN "MPN"
	1    8950 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	8950 4700 8950 4750
Wire Wire Line
	8950 4750 9300 4750
Wire Wire Line
	9300 4600 9300 4750
Connection ~ 8950 4750
Wire Wire Line
	8950 4750 8950 4800
$Comp
L power:GND #PWR027
U 1 1 5D6595E8
P 8950 5050
F 0 "#PWR027" H 8950 4800 50  0001 C CNN
F 1 "GND" H 8955 4877 50  0000 C CNN
F 2 "" H 8950 5050 50  0001 C CNN
F 3 "" H 8950 5050 50  0001 C CNN
	1    8950 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	8950 5050 8950 5000
$Comp
L Device:Crystal_GND24 Y2
U 1 1 5D752164
P 13450 3200
F 0 "Y2" V 13350 2900 50  0000 L CNN
F 1 "32MHz" V 13250 2900 50  0000 L CNN
F 2 "Crystal:Crystal_SMD_2016-4Pin_2.0x1.6mm" H 13450 3200 50  0001 C CNN
F 3 "~" H 13450 3200 50  0001 C CNN
F 4 "1253-1118-1-ND" H 13450 3200 50  0001 C CNN "DKPN"
F 5 "CX2016DB32000D0FLJCC" H 13450 3200 50  0001 C CNN "MPN"
	1    13450 3200
	0    1    1    0   
$EndComp
$Comp
L Device:C_Small C18
U 1 1 5D7538AB
P 13150 3350
F 0 "C18" H 12950 3400 50  0000 L CNN
F 1 "12pF" H 12900 3300 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 13150 3350 50  0001 C CNN
F 3 "~" H 13150 3350 50  0001 C CNN
F 4 "399-1013-1-ND" H 13150 3350 50  0001 C CNN "DKPN"
F 5 "C0402C120J5GACTU" H 13150 3350 50  0001 C CNN "MPN"
	1    13150 3350
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C19
U 1 1 5D754208
P 13850 3050
F 0 "C19" H 13942 3096 50  0000 L CNN
F 1 "12pF" H 13942 3005 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 13850 3050 50  0001 C CNN
F 3 "~" H 13850 3050 50  0001 C CNN
F 4 "399-1013-1-ND" H 13850 3050 50  0001 C CNN "DKPN"
F 5 "C0402C120J5GACTU" H 13850 3050 50  0001 C CNN "MPN"
	1    13850 3050
	1    0    0    -1  
$EndComp
Wire Wire Line
	13450 3050 13450 2900
Wire Wire Line
	13450 2900 12650 2900
Wire Wire Line
	12650 2900 12650 3000
Wire Wire Line
	13450 3350 13450 3500
Wire Wire Line
	13450 3500 13150 3500
Wire Wire Line
	12650 3500 12650 3100
Wire Wire Line
	13150 3450 13150 3500
Connection ~ 13150 3500
Wire Wire Line
	13150 3500 12650 3500
Wire Wire Line
	13250 3200 13150 3200
Wire Wire Line
	13150 3200 13150 3250
Wire Wire Line
	13650 3200 13850 3200
Wire Wire Line
	13850 3200 13850 3150
Wire Wire Line
	13850 2900 13850 2950
Wire Wire Line
	13450 2900 13850 2900
Connection ~ 13450 2900
$Comp
L power:GND #PWR035
U 1 1 5D924FE6
P 13050 3200
F 0 "#PWR035" H 13050 2950 50  0001 C CNN
F 1 "GND" H 13055 3027 50  0000 C CNN
F 2 "" H 13050 3200 50  0001 C CNN
F 3 "" H 13050 3200 50  0001 C CNN
	1    13050 3200
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR037
U 1 1 5D92544E
P 13950 3200
F 0 "#PWR037" H 13950 2950 50  0001 C CNN
F 1 "GND" H 13955 3027 50  0000 C CNN
F 2 "" H 13950 3200 50  0001 C CNN
F 3 "" H 13950 3200 50  0001 C CNN
	1    13950 3200
	0    -1   -1   0   
$EndComp
Wire Wire Line
	13850 3200 13950 3200
Connection ~ 13850 3200
Wire Wire Line
	13050 3200 13150 3200
Connection ~ 13150 3200
$Comp
L Device:C_Small C14
U 1 1 5D9EB882
P 9700 1300
F 0 "C14" H 9792 1346 50  0000 L CNN
F 1 "1uF" H 9792 1255 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 9700 1300 50  0001 C CNN
F 3 "~" H 9700 1300 50  0001 C CNN
F 4 "311-1438-1-ND" H 9700 1300 50  0001 C CNN "DKPN"
F 5 "CC0402KRX5R5BB105" H 9700 1300 50  0001 C CNN "MPN"
	1    9700 1300
	1    0    0    -1  
$EndComp
$Comp
L Device:L_Small L1
U 1 1 5D9F37BA
P 9250 1300
F 0 "L1" H 9298 1346 50  0000 L CNN
F 1 "15nH" H 9298 1255 50  0000 L CNN
F 2 "Inductor_SMD:L_0402_1005Metric" H 9250 1300 50  0001 C CNN
F 3 "~" H 9250 1300 50  0001 C CNN
F 4 "587-1521-1-ND‎" H 9250 1300 50  0001 C CNN "DKPN"
F 5 "HK100515NJ-T‎" H 9250 1300 50  0001 C CNN "MPN"
	1    9250 1300
	1    0    0    -1  
$EndComp
$Comp
L Device:L_Small L2
U 1 1 5D9F3F40
P 9250 1550
F 0 "L2" H 9298 1596 50  0000 L CNN
F 1 "10uH" H 9298 1505 50  0000 L CNN
F 2 "Inductor_SMD:L_0603_1608Metric" H 9250 1550 50  0001 C CNN
F 3 "~" H 9250 1550 50  0001 C CNN
F 4 "490-4025-1-ND‎" H 9250 1550 50  0001 C CNN "DKPN"
F 5 "‎LQM18FN100M00D‎" H 9250 1550 50  0001 C CNN "MPN"
	1    9250 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	8950 1850 8950 1950
Wire Wire Line
	9300 1950 8950 1950
Connection ~ 8950 1950
Wire Wire Line
	8950 1950 8950 2000
Wire Wire Line
	9250 1400 9250 1450
Wire Wire Line
	9250 1200 9250 1150
Wire Wire Line
	9250 1150 9500 1150
Wire Wire Line
	9700 1150 9700 1200
Wire Wire Line
	9250 1650 9250 1750
Wire Wire Line
	9250 1750 9400 1750
Connection ~ 9500 1150
Wire Wire Line
	9500 1150 9700 1150
Wire Wire Line
	9700 1500 9700 1450
Wire Wire Line
	9700 1450 9600 1450
Connection ~ 9700 1450
Wire Wire Line
	9700 1450 9700 1400
$Comp
L Connector:Conn_Coaxial J4
U 1 1 5DD824B7
P 14200 3800
F 0 "J4" H 14300 3775 50  0000 L CNN
F 1 "RP_SMA" H 14300 3684 50  0000 L CNN
F 2 "Connector_Coaxial:SMA_Samtec_SMA-J-P-H-ST-EM1_EdgeMount" H 14200 3800 50  0001 C CNN
F 3 " ~" H 14200 3800 50  0001 C CNN
F 4 "CONREVSMA003.062-ND" H 14200 3800 50  0001 C CNN "DKPN"
F 5 "CONREVSMA003.062" H 14200 3800 50  0001 C CNN "MPN"
	1    14200 3800
	1    0    0    -1  
$EndComp
$Comp
L alvarop:2450FM07A0029T F1
U 1 1 5DD835CC
P 12600 3850
F 0 "F1" H 12600 4147 60  0000 C CNN
F 1 "2450FM07A0029T" H 12600 4041 60  0000 C CNN
F 2 "alvarop:2450FM07A0029T" H 12600 3850 60  0001 C CNN
F 3 "" H 12600 3850 60  0001 C CNN
F 4 "712-1623-1-ND" H 12600 3850 50  0001 C CNN "DKPN"
F 5 "2450FM07A0029T" H 12600 3850 50  0001 C CNN "MPN"
	1    12600 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	11000 3500 11500 3500
Wire Wire Line
	11500 3500 11500 3800
Wire Wire Line
	11500 3800 12200 3800
Wire Wire Line
	11900 3300 11900 3350
Wire Wire Line
	11000 3300 11900 3300
Wire Wire Line
	11000 3200 12100 3200
Wire Wire Line
	11600 3400 11600 3700
Wire Wire Line
	11600 3700 12050 3700
Wire Wire Line
	12050 3700 12050 3900
Wire Wire Line
	12050 3900 12200 3900
Wire Wire Line
	11000 3400 11600 3400
$Comp
L power:GND #PWR032
U 1 1 5DEAB976
P 12050 3950
F 0 "#PWR032" H 12050 3700 50  0001 C CNN
F 1 "GND" H 12055 3777 50  0000 C CNN
F 2 "" H 12050 3950 50  0001 C CNN
F 3 "" H 12050 3950 50  0001 C CNN
	1    12050 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	12050 3950 12050 3900
Connection ~ 12050 3900
$Comp
L power:GND #PWR036
U 1 1 5DEDA4CC
P 13100 3950
F 0 "#PWR036" H 13100 3700 50  0001 C CNN
F 1 "GND" H 13105 3777 50  0000 C CNN
F 2 "" H 13100 3950 50  0001 C CNN
F 3 "" H 13100 3950 50  0001 C CNN
	1    13100 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	13100 3950 13100 3900
Wire Wire Line
	13100 3900 13000 3900
$Comp
L power:GND #PWR038
U 1 1 5DF3C069
P 14200 4050
F 0 "#PWR038" H 14200 3800 50  0001 C CNN
F 1 "GND" H 14205 3877 50  0000 C CNN
F 2 "" H 14200 4050 50  0001 C CNN
F 3 "" H 14200 4050 50  0001 C CNN
	1    14200 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	14200 4000 14200 4050
Wire Wire Line
	11000 3000 12650 3000
Wire Wire Line
	11000 3100 12650 3100
NoConn ~ 9900 4600
$Comp
L power:PWR_FLAG #FLG02
U 1 1 5E50905B
P 9500 1150
F 0 "#FLG02" H 9500 1225 50  0001 C CNN
F 1 "PWR_FLAG" H 9500 1323 50  0000 C CNN
F 2 "" H 9500 1150 50  0001 C CNN
F 3 "~" H 9500 1150 50  0001 C CNN
	1    9500 1150
	1    0    0    -1  
$EndComp
Wire Wire Line
	2450 3050 2650 3050
Wire Wire Line
	2100 3050 2450 3050
Connection ~ 2450 3050
Wire Wire Line
	2450 3100 2450 3050
$Comp
L power:PWR_FLAG #FLG01
U 1 1 5D5441BD
P 2450 3100
F 0 "#FLG01" H 2450 3175 50  0001 C CNN
F 1 "PWR_FLAG" H 2450 3273 50  0000 C CNN
F 2 "" H 2450 3100 50  0001 C CNN
F 3 "~" H 2450 3100 50  0001 C CNN
	1    2450 3100
	-1   0    0    1   
$EndComp
Text Label 9500 5150 1    50   ~ 0
RX_LED
Wire Wire Line
	9500 4600 9500 5150
Wire Wire Line
	9400 4600 9400 5150
Text Label 9400 5150 1    50   ~ 0
TX_LED
NoConn ~ 10100 4600
NoConn ~ 10000 4600
Wire Wire Line
	9200 2200 9200 2300
$Comp
L power:GND #PWR028
U 1 1 5E7B3C2F
P 9200 2200
F 0 "#PWR028" H 9200 1950 50  0001 C CNN
F 1 "GND" H 9205 2027 50  0000 C CNN
F 2 "" H 9200 2200 50  0001 C CNN
F 3 "" H 9200 2200 50  0001 C CNN
	1    9200 2200
	-1   0    0    1   
$EndComp
NoConn ~ 8700 3200
$Comp
L Device:C_Small C9
U 1 1 5DD247F3
P 7500 2850
F 0 "C9" H 7300 2900 50  0000 L CNN
F 1 "12pF" H 7250 2800 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 7500 2850 50  0001 C CNN
F 3 "~" H 7500 2850 50  0001 C CNN
F 4 "399-1013-1-ND" H 7500 2850 50  0001 C CNN "DKPN"
F 5 "C0402C120J5GACTU" H 7500 2850 50  0001 C CNN "MPN"
	1    7500 2850
	0    1    1    0   
$EndComp
$Comp
L Device:C_Small C10
U 1 1 5DD223CD
P 7500 3250
F 0 "C10" H 7300 3300 50  0000 L CNN
F 1 "12pF" H 7250 3200 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 7500 3250 50  0001 C CNN
F 3 "~" H 7500 3250 50  0001 C CNN
F 4 "399-1013-1-ND" H 7500 3250 50  0001 C CNN "DKPN"
F 5 "C0402C120J5GACTU" H 7500 3250 50  0001 C CNN "MPN"
	1    7500 3250
	0    1    1    0   
$EndComp
Wire Wire Line
	9600 1450 9600 2300
Wire Wire Line
	9500 2300 9500 1150
Wire Wire Line
	9400 1750 9400 2300
Wire Wire Line
	9300 1950 9300 2300
Wire Wire Line
	8700 3100 7900 3100
Wire Wire Line
	8700 3000 7900 3000
Wire Wire Line
	8150 2700 8150 2750
$Comp
L power:GND #PWR023
U 1 1 5D3BE23F
P 8150 2750
F 0 "#PWR023" H 8150 2500 50  0001 C CNN
F 1 "GND" H 8155 2577 50  0000 C CNN
F 2 "" H 8150 2750 50  0001 C CNN
F 3 "" H 8150 2750 50  0001 C CNN
	1    8150 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	8400 2900 8700 2900
Wire Wire Line
	8400 2450 8400 2900
Wire Wire Line
	8150 2450 8400 2450
Wire Wire Line
	8150 2500 8150 2450
Wire Wire Line
	8950 2200 8950 2250
$Comp
L power:GND #PWR025
U 1 1 5D35FA9B
P 8950 2250
F 0 "#PWR025" H 8950 2000 50  0001 C CNN
F 1 "GND" H 8955 2077 50  0000 C CNN
F 2 "" H 8950 2250 50  0001 C CNN
F 3 "" H 8950 2250 50  0001 C CNN
	1    8950 2250
	1    0    0    -1  
$EndComp
$Comp
L alvarop:NRF52811-QFAA U3
U 1 1 5CE6B290
P 9850 3450
F 0 "U3" H 8250 4750 60  0000 L CNN
F 1 "NRF52811-QFAA" H 8100 4650 60  0000 L CNN
F 2 "Package_DFN_QFN:QFN-48-1EP_6x6mm_P0.4mm_EP4.6x4.6mm" H 8700 4000 50  0001 C CNN
F 3 "" H 8700 4000 50  0001 C CNN
F 4 "‎1490-1075-1-ND‎" H 9850 3450 50  0001 C CNN "DKPN"
F 5 " ‎NRF52811-QFAA-R‎" H 9850 3450 50  0001 C CNN "MPN"
	1    9850 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	7900 3100 7900 3250
Wire Wire Line
	7900 2850 7700 2850
Wire Wire Line
	7900 3000 7900 2850
Wire Wire Line
	7900 3250 7700 3250
Wire Wire Line
	7700 3250 7600 3250
Connection ~ 7700 3250
Wire Wire Line
	7700 3150 7700 3250
Wire Wire Line
	7700 2850 7700 2950
Connection ~ 7700 2850
Wire Wire Line
	7600 2850 7700 2850
$Comp
L Device:Crystal_Small Y1
U 1 1 5C2FC751
P 7700 3050
F 0 "Y1" V 7350 3050 50  0000 L CNN
F 1 "32.768kHz" V 7450 3050 50  0000 L CNN
F 2 "Crystal:Crystal_SMD_3215-2Pin_3.2x1.5mm" H 7700 3050 50  0001 C CNN
F 3 "~" H 7700 3050 50  0001 C CNN
F 4 "‎SER4084CT-ND‎" H 7700 3050 50  0001 C CNN "DKPN"
F 5 "FC-135 32.7680KA-AC0" H 7700 3050 50  0001 C CNN "MPN"
	1    7700 3050
	0    1    1    0   
$EndComp
$Comp
L Device:C_Small C11
U 1 1 5C2FC2A3
P 8150 2600
F 0 "C11" H 8000 2750 50  0000 L CNN
F 1 "0.1uF" H 7950 2500 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 8150 2600 50  0001 C CNN
F 3 "~" H 8150 2600 50  0001 C CNN
F 4 "490-1318-1-ND" H 8150 2600 50  0001 C CNN "DKPN"
F 5 "GRM155R61A104KA01D" H 8150 2600 50  0001 C CNN "MPN"
	1    8150 2600
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C12
U 1 1 5C2FBEB8
P 8950 2100
F 0 "C12" H 8750 2200 50  0000 L CNN
F 1 "4.7uF" H 8700 2000 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 8950 2100 50  0001 C CNN
F 3 "~" H 8950 2100 50  0001 C CNN
F 4 "1276-2087-1-ND" H 300 400 50  0001 C CNN "DKPN"
F 5 "CL10B475KQ8NQNC" H 300 400 50  0001 C CNN "MPN"
	1    8950 2100
	1    0    0    -1  
$EndComp
NoConn ~ 11000 3600
NoConn ~ 11000 3700
NoConn ~ 11000 3800
NoConn ~ 10200 4600
NoConn ~ 10300 4600
NoConn ~ 8700 3600
NoConn ~ 8700 3300
Wire Wire Line
	13000 3800 14000 3800
$Comp
L alvarop:FT234XD U2
U 1 1 5CF5B71C
P 5700 6800
F 0 "U2" H 5700 7147 60  0000 C CNN
F 1 "FT234XD" H 5700 7041 60  0000 C CNN
F 2 "alvarop:FTDI_DFN12" H 5700 6800 60  0001 C CNN
F 3 "" H 5700 6800 60  0001 C CNN
F 4 "FT234XD-R" H 5700 6800 50  0001 C CNN "DKPN"
F 5 "768-1178-1-ND" H 5700 6800 50  0001 C CNN "MPN"
	1    5700 6800
	1    0    0    -1  
$EndComp
$Comp
L Connector:USB_A J1
U 1 1 5CF5FFBB
P 1500 7250
F 0 "J1" H 1557 7717 50  0000 C CNN
F 1 "USB_A" H 1557 7626 50  0000 C CNN
F 2 "alvarop:MOLEX-0480371000" H 1650 7200 50  0001 C CNN
F 3 " ~" H 1650 7200 50  0001 C CNN
F 4 "WM17118-ND" H 1500 7250 50  0001 C CNN "DKPN"
F 5 "0480371000" H 1500 7250 50  0001 C CNN "MPN"
	1    1500 7250
	1    0    0    -1  
$EndComp
$Comp
L Device:Ferrite_Bead_Small FB1
U 1 1 5CF6164C
P 2350 7050
F 0 "FB1" V 2587 7050 50  0000 C CNN
F 1 "FB" V 2496 7050 50  0000 C CNN
F 2 "Inductor_SMD:L_0402_1005Metric" V 2280 7050 50  0001 C CNN
F 3 "~" H 2350 7050 50  0001 C CNN
F 4 "490-1006-1-ND" H 2350 7050 50  0001 C CNN "DKPN"
F 5 "BLM15AG601SN1D" H 2350 7050 50  0001 C CNN "MPN"
	1    2350 7050
	0    -1   -1   0   
$EndComp
$Comp
L Device:C_Small C3
U 1 1 5CF6C134
P 1950 6900
F 0 "C3" H 2042 6946 50  0000 L CNN
F 1 "10nF" H 2042 6855 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 1950 6900 50  0001 C CNN
F 3 "~" H 1950 6900 50  0001 C CNN
F 4 "1276-1028-1-ND" H 1950 6900 50  0001 C CNN "DKPN"
F 5 "CL05B103KB5NNNC" H 1950 6900 50  0001 C CNN "MPN"
	1    1950 6900
	1    0    0    -1  
$EndComp
Wire Wire Line
	1800 7250 1850 7250
Wire Wire Line
	1800 7350 2150 7350
Text Label 2500 7250 2    50   ~ 0
USB_DM
Text Label 2500 7350 2    50   ~ 0
USB_DP
$Comp
L Device:R_Small R2
U 1 1 5CF83F6E
P 4950 6700
F 0 "R2" V 4754 6700 50  0000 C CNN
F 1 "27" V 4850 6700 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" H 4950 6700 50  0001 C CNN
F 3 "~" H 4950 6700 50  0001 C CNN
F 4 "311-27JRCT-ND" H 4950 6700 50  0001 C CNN "DKPN"
F 5 "RC0402JR-0727RL" H 4950 6700 50  0001 C CNN "MPN"
	1    4950 6700
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R6
U 1 1 5CF85062
P 6450 6700
F 0 "R6" V 6254 6700 50  0000 C CNN
F 1 "27" V 6350 6700 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" H 6450 6700 50  0001 C CNN
F 3 "~" H 6450 6700 50  0001 C CNN
F 4 "311-27JRCT-ND" H 6450 6700 50  0001 C CNN "DKPN"
F 5 "RC0402JR-0727RL" H 6450 6700 50  0001 C CNN "MPN"
	1    6450 6700
	0    1    1    0   
$EndComp
NoConn ~ 5150 6900
NoConn ~ 5150 7200
NoConn ~ 6250 7100
NoConn ~ 6250 6800
Wire Wire Line
	5050 6700 5150 6700
Wire Wire Line
	6250 6700 6350 6700
Wire Wire Line
	6550 6700 6900 6700
Wire Wire Line
	4850 6700 4500 6700
Text Label 4500 6700 0    50   ~ 0
USB_DM
Text Label 6900 6700 2    50   ~ 0
USB_DP
Wire Wire Line
	1950 7000 1950 7050
Wire Wire Line
	1950 7050 1800 7050
Wire Wire Line
	1950 7050 2250 7050
Connection ~ 1950 7050
$Comp
L Device:C_Small C6
U 1 1 5CFD3D3C
P 2800 7250
F 0 "C6" H 2650 7400 50  0000 L CNN
F 1 "0.1uF" H 2600 7150 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 2800 7250 50  0001 C CNN
F 3 "~" H 2800 7250 50  0001 C CNN
F 4 "490-1318-1-ND" H 2800 7250 50  0001 C CNN "DKPN"
F 5 "GRM155R61A104KA01D" H 2800 7250 50  0001 C CNN "MPN"
	1    2800 7250
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C8
U 1 1 5CFD45D8
P 5700 3750
F 0 "C8" H 5550 3900 50  0000 L CNN
F 1 "0.1uF" H 5500 3650 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 5700 3750 50  0001 C CNN
F 3 "~" H 5700 3750 50  0001 C CNN
F 4 "490-1318-1-ND" H 5700 3750 50  0001 C CNN "DKPN"
F 5 "GRM155R61A104KA01D" H 5700 3750 50  0001 C CNN "MPN"
	1    5700 3750
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C7
U 1 1 5CFE40D4
P 3100 7250
F 0 "C7" H 2900 7350 50  0000 L CNN
F 1 "4.7uF" H 2850 7150 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3100 7250 50  0001 C CNN
F 3 "~" H 3100 7250 50  0001 C CNN
F 4 "1276-2087-1-ND" H -5550 5550 50  0001 C CNN "DKPN"
F 5 "CL10B475KQ8NQNC" H -5550 5550 50  0001 C CNN "MPN"
	1    3100 7250
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C2
U 1 1 5CFE6D5F
P 1850 7500
F 0 "C2" H 1942 7546 50  0000 L CNN
F 1 "47pF" H 1942 7455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 1850 7500 50  0001 C CNN
F 3 "~" H 1850 7500 50  0001 C CNN
F 4 "399-7796-1-ND" H 1850 7500 50  0001 C CNN "DKPN"
F 5 "C0402C470K5GACTU" H 1850 7500 50  0001 C CNN "MPN"
	1    1850 7500
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C4
U 1 1 5CFE7092
P 2150 7500
F 0 "C4" H 2242 7546 50  0000 L CNN
F 1 "47pF" H 2242 7455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0402_1005Metric" H 2150 7500 50  0001 C CNN
F 3 "~" H 2150 7500 50  0001 C CNN
F 4 "399-7796-1-ND" H 2150 7500 50  0001 C CNN "DKPN"
F 5 "C0402C470K5GACTU" H 2150 7500 50  0001 C CNN "MPN"
	1    2150 7500
	1    0    0    -1  
$EndComp
Wire Wire Line
	1850 7400 1850 7250
Connection ~ 1850 7250
Wire Wire Line
	1850 7250 2500 7250
Wire Wire Line
	2150 7400 2150 7350
Connection ~ 2150 7350
Wire Wire Line
	2150 7350 2500 7350
$Comp
L power:GND #PWR02
U 1 1 5D05BC6D
P 1500 7750
F 0 "#PWR02" H 1500 7500 50  0001 C CNN
F 1 "GND" H 1505 7577 50  0000 C CNN
F 2 "" H 1500 7750 50  0001 C CNN
F 3 "" H 1500 7750 50  0001 C CNN
	1    1500 7750
	1    0    0    -1  
$EndComp
Wire Wire Line
	1500 7750 1500 7650
$Comp
L power:GND #PWR04
U 1 1 5D063A0F
P 1950 6700
F 0 "#PWR04" H 1950 6450 50  0001 C CNN
F 1 "GND" H 1955 6527 50  0000 C CNN
F 2 "" H 1950 6700 50  0001 C CNN
F 3 "" H 1950 6700 50  0001 C CNN
	1    1950 6700
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR03
U 1 1 5D063E1E
P 1850 7700
F 0 "#PWR03" H 1850 7450 50  0001 C CNN
F 1 "GND" H 1855 7527 50  0000 C CNN
F 2 "" H 1850 7700 50  0001 C CNN
F 3 "" H 1850 7700 50  0001 C CNN
	1    1850 7700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR06
U 1 1 5D0640F6
P 2150 7700
F 0 "#PWR06" H 2150 7450 50  0001 C CNN
F 1 "GND" H 2155 7527 50  0000 C CNN
F 2 "" H 2150 7700 50  0001 C CNN
F 3 "" H 2150 7700 50  0001 C CNN
	1    2150 7700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR08
U 1 1 5D0643CC
P 2800 7450
F 0 "#PWR08" H 2800 7200 50  0001 C CNN
F 1 "GND" H 2805 7277 50  0000 C CNN
F 2 "" H 2800 7450 50  0001 C CNN
F 3 "" H 2800 7450 50  0001 C CNN
	1    2800 7450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR09
U 1 1 5D0647E6
P 3100 7450
F 0 "#PWR09" H 3100 7200 50  0001 C CNN
F 1 "GND" H 3105 7277 50  0000 C CNN
F 2 "" H 3100 7450 50  0001 C CNN
F 3 "" H 3100 7450 50  0001 C CNN
	1    3100 7450
	1    0    0    -1  
$EndComp
Wire Wire Line
	1950 6700 1950 6800
Wire Wire Line
	2450 7050 2800 7050
Wire Wire Line
	2800 7150 2800 7050
Connection ~ 2800 7050
Wire Wire Line
	2800 7050 3100 7050
Wire Wire Line
	3100 7150 3100 7050
Connection ~ 3100 7050
Wire Wire Line
	3100 7050 3450 7050
Wire Wire Line
	2800 7450 2800 7350
Wire Wire Line
	3100 7450 3100 7350
Text Label 3450 7050 2    50   ~ 0
VIN
Text Label 4900 7000 0    50   ~ 0
VIN
Wire Wire Line
	4900 7000 5050 7000
Wire Wire Line
	5150 7100 4900 7100
Wire Wire Line
	4900 7100 4900 7200
$Comp
L power:GND #PWR011
U 1 1 5D0CD175
P 4900 7200
F 0 "#PWR011" H 4900 6950 50  0001 C CNN
F 1 "GND" H 4905 7027 50  0000 C CNN
F 2 "" H 4900 7200 50  0001 C CNN
F 3 "" H 4900 7200 50  0001 C CNN
	1    4900 7200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 6800 5050 6800
Wire Wire Line
	5050 6800 5050 7000
Connection ~ 5050 7000
Wire Wire Line
	5050 7000 5150 7000
$Comp
L power:+3.3V #PWR018
U 1 1 5D0D6B7B
P 6650 7000
F 0 "#PWR018" H 6650 6850 50  0001 C CNN
F 1 "+3.3V" H 6665 7173 50  0000 C CNN
F 2 "" H 6650 7000 50  0001 C CNN
F 3 "" H 6650 7000 50  0001 C CNN
	1    6650 7000
	0    1    1    0   
$EndComp
Wire Wire Line
	6650 7000 6250 7000
Wire Wire Line
	6250 6900 6650 6900
Wire Wire Line
	6250 7200 6650 7200
Text Label 6650 6900 2    50   ~ 0
UART_TX
Text Label 6650 7200 2    50   ~ 0
UART_RX
$Comp
L Device:R_Small R5
U 1 1 5D12FBFB
P 5950 5600
F 0 "R5" V 6050 5650 50  0000 R CNN
F 1 "10k" V 5850 5650 50  0000 R CNN
F 2 "Resistor_SMD:R_0402_1005Metric" H 5950 5600 50  0001 C CNN
F 3 "~" H 5950 5600 50  0001 C CNN
F 4 "311-10KJRCT-ND" H 3700 -4600 50  0001 C CNN "DKPN"
F 5 "RC0402JR-0710KL" H 3700 -4600 50  0001 C CNN "MPN"
	1    5950 5600
	0    -1   -1   0   
$EndComp
$Comp
L Device:LED_ALT D3
U 1 1 5D12FC03
P 5550 5600
F 0 "D3" H 5541 5816 50  0000 C CNN
F 1 "LED_ALT" H 5541 5725 50  0000 C CNN
F 2 "LED_SMD:LED_0402_1005Metric" H 5550 5600 50  0001 C CNN
F 3 "~" H 5550 5600 50  0001 C CNN
F 4 "1516-1216-1-ND" H 0   1100 50  0001 C CNN "DKPN"
F 5 "QBLP595-IG" H 0   1100 50  0001 C CNN "MPN"
	1    5550 5600
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 5600 5850 5600
Wire Wire Line
	6050 5600 6550 5600
Wire Wire Line
	5400 5600 5200 5600
$Comp
L power:GND #PWR016
U 1 1 5D12FC0C
P 5200 5650
F 0 "#PWR016" H 5200 5400 50  0001 C CNN
F 1 "GND" H 5205 5477 50  0000 C CNN
F 2 "" H 5200 5650 50  0001 C CNN
F 3 "" H 5200 5650 50  0001 C CNN
	1    5200 5650
	1    0    0    -1  
$EndComp
Text Label 6550 5600 2    50   ~ 0
STATUS_LED
Wire Wire Line
	5200 5600 5200 5650
Text Label 9600 5150 1    50   ~ 0
STATUS_LED
Wire Wire Line
	9600 4600 9600 5150
NoConn ~ 9700 4600
NoConn ~ 9800 4600
NoConn ~ 8700 3700
NoConn ~ 8700 3500
NoConn ~ 8700 3400
NoConn ~ 9800 2300
NoConn ~ 9900 2300
NoConn ~ 10000 2300
NoConn ~ 10100 2300
NoConn ~ 10200 2300
NoConn ~ 10300 2300
NoConn ~ 10400 2300
Wire Wire Line
	1850 7600 1850 7700
Wire Wire Line
	2150 7600 2150 7700
$Comp
L power:GND #PWR01
U 1 1 5D1F6B33
P 1400 8100
F 0 "#PWR01" H 1400 7850 50  0001 C CNN
F 1 "GND" H 1405 7927 50  0000 C CNN
F 2 "" H 1400 8100 50  0001 C CNN
F 3 "" H 1400 8100 50  0001 C CNN
	1    1400 8100
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R1
U 1 1 5D1F6ED8
P 1400 7900
F 0 "R1" V 1204 7900 50  0000 C CNN
F 1 "DNP" V 1300 7900 50  0000 C CNN
F 2 "Resistor_SMD:R_0402_1005Metric" H 1400 7900 50  0001 C CNN
F 3 "~" H 1400 7900 50  0001 C CNN
	1    1400 7900
	1    0    0    -1  
$EndComp
Wire Wire Line
	1400 7800 1400 7650
Wire Wire Line
	1400 8100 1400 8000
$Comp
L Connector:TestPoint TP2
U 1 1 5D271B59
P 7500 4650
F 0 "TP2" V 7454 4838 50  0000 L CNN
F 1 "RX" V 7545 4838 50  0000 L CNN
F 2 "TestPoint:TestPoint_Pad_D1.5mm" H 7700 4650 50  0001 C CNN
F 3 "~" H 7700 4650 50  0001 C CNN
	1    7500 4650
	0    1    1    0   
$EndComp
$Comp
L Connector:TestPoint TP1
U 1 1 5D272E09
P 7500 4450
F 0 "TP1" V 7454 4638 50  0000 L CNN
F 1 "TX" V 7545 4638 50  0000 L CNN
F 2 "TestPoint:TestPoint_Pad_D1.5mm" H 7700 4450 50  0001 C CNN
F 3 "~" H 7700 4450 50  0001 C CNN
	1    7500 4450
	0    1    1    0   
$EndComp
$Comp
L Connector:TestPoint TP3
U 1 1 5D275EFD
P 7500 4850
F 0 "TP3" V 7454 5038 50  0000 L CNN
F 1 "GND" V 7545 5038 50  0000 L CNN
F 2 "TestPoint:TestPoint_Pad_D1.5mm" H 7700 4850 50  0001 C CNN
F 3 "~" H 7700 4850 50  0001 C CNN
	1    7500 4850
	0    1    1    0   
$EndComp
$Comp
L Connector:TestPoint TP4
U 1 1 5D275F03
P 7500 5050
F 0 "TP4" V 7454 5238 50  0000 L CNN
F 1 "3V3" V 7545 5238 50  0000 L CNN
F 2 "TestPoint:TestPoint_Pad_D1.5mm" H 7700 5050 50  0001 C CNN
F 3 "~" H 7700 5050 50  0001 C CNN
	1    7500 5050
	0    1    1    0   
$EndComp
Text Label 7100 4450 0    50   ~ 0
UART_TX
Wire Wire Line
	7100 4450 7500 4450
Text Label 7100 4650 0    50   ~ 0
UART_RX
Wire Wire Line
	7100 4650 7500 4650
$Comp
L power:GND #PWR0101
U 1 1 5D34C3EB
P 7350 4850
F 0 "#PWR0101" H 7350 4600 50  0001 C CNN
F 1 "GND" H 7355 4677 50  0000 C CNN
F 2 "" H 7350 4850 50  0001 C CNN
F 3 "" H 7350 4850 50  0001 C CNN
	1    7350 4850
	0    1    1    0   
$EndComp
Wire Wire Line
	7350 4850 7500 4850
$Comp
L power:+3.3V #PWR0102
U 1 1 5D3564B9
P 7350 5050
F 0 "#PWR0102" H 7350 4900 50  0001 C CNN
F 1 "+3.3V" H 7365 5223 50  0000 C CNN
F 2 "" H 7350 5050 50  0001 C CNN
F 3 "" H 7350 5050 50  0001 C CNN
	1    7350 5050
	0    -1   -1   0   
$EndComp
Wire Wire Line
	7350 5050 7500 5050
$EndSCHEMATC
