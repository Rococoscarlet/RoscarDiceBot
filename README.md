# RoscarDiceBot
A CoC TRPG dice bot application for CoolQ

Introduce
--------
This project is an QQ bot.
With it you may roll dice and do simple calculate with simple commands in QQ.

This project is an plugin for CoolQ.
To use the application, at least a CoolQ Air program is required.
You may get it for free at its official website.
https://cqp.cc

This project is built based on CoolQ C++ SDK. 
https://github.com/CoolQ/cqsdk-vc

Thanks to the note from 铮铮卡穆 in CoolQ community.
https://cqp.cc/t/28730

Document
--------
`.r {expression} {?reason} {?DC}`

	For example:
	`.r 2d6+1 sanlose 5`
	Bot may response:
	`你对sanlose检定骰出(5)+1=6, DC:5`
	or`You got (5+3)+1=9 for sanlose, DC:5`

	In expression, you can use four arithmetic operations, brackets, and most
	importantly the dice operator `d`.

	`d` has two parameter which both can be omitted. 
	The number before `d` is the quantity of dices ( defaultly 1 ).
	The number after `d` is the faces of dices ( defaultly 100 ).

	All dices will be showed in response.

	`d` has an higher priority than brackets, for the sake of clear convey.
	That means `d` should be directly connected with its parameter.
	
	Further more, you can add reward dice or punishment dice to d100
	according to CoC v7.
	`d^100` means a reward dice and `d__100` means two punishment dice and so on.

`.h {expression} {?reason} {?DC}`
	
	Almostly the same as `.r`, except that it's a hiden rolling.
	Use `.h` in group, and result will be privately informed.

`.c {expression}` (Will be soon realized)

	Just calculate mode.
	Use it as an convinient calculater.
	Dice operator not recommanded in this mode, but of cause you can use it.

`.build {version} {name}` (Will be soon realized)

	`.build CoC7 Jack` will build a random character based on CoC V7 rule called Jack.
	`.build CoC6 Jack` will build a CoC V6 one.
	Other version may be added or not.

Prospection
--------
A log function will be added at version 1.1.0.
You will be able to get a pretty log when you play TRPG with friends in QQ group.

A public bot run on my server may be offered soon, 
if you don't want to run it on your own computer.

Any other suggestions are welcome.