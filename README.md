# RoscarDiceBot
A CoC TRPG dice bot application for CoolQ

Introduce
--------
This project is an QQ bot.
With it you may roll dice and do simple calculate with simple commands in QQ.

This project is an plug-in for CoolQ.
To use the application, at least a CoolQ Air program is required.
You may get it for free at its [official website](https://cqp.cc).

This project is built based on [CoolQ C++ SDK](https://github.com/CoolQ/cqsdk-vc). 

Thanks to the note from [铮铮卡穆](https://cqp.cc/t/28730) in CoolQ community.


Document
--------
*	`.r {expression} {?reason} {?DC}`
	
	For example:
	`.r 2d6+1 SanLose 5`
	Bot may response:
	`你对SanLose检定骰出(5+3)+1=9, DC:5`
	or`You got (5+3)+1=9 for SanLose, DC:5`
	
	In expression, you can use four arithmetic operations, brackets, and most importantly the dice operator `d`.
	
	`d` has two parameter which both can be omitted. 
	The number before `d` is the quantity of dices ( default 1 ).
	The number after `d` is the faces of dices ( default 100 ).
	
	All dices will be showed in response.
	
	Further more, you can add bonus dice or penalty dice to d100 according to CoC v7.
	`d^100` means a bonus dice and `d__100` means two penalty dice and so on.
	
*	`.h {expression} {?reason} {?DC}`
		
	Almost the same as `.r`, except that it's a hidden rolling.
	Use `.h` in group, and result will be privately informed.
	
*	`.c {expression}`
	
	Just calculate mode.
	Use it as an convenient calculator.
	Dice operator not recommended in this mode, but of course you can use it.
	
*	`.luck`
	Test your luck today.
	
*	`.build {version} {name}` (Will be soon realized)
	
	`.build CoC7 Jack` will build a random character based on CoC V7 rule called Jack.
	`.build CoC6 Jack` will build a CoC V6 one.
	Other version may be added sometime or not.

TODO
--------
A log function will be added at version 1.1.0.
You will be able to get a pretty log when you play TRPG with friends in QQ group.

A public bot run on my server may be offered soon, 
if you don't want to run it on your own computer.

Any other suggestions are welcome.