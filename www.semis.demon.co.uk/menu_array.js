/*
 DHTML Menu version 3.3.19
 Written by Andy Woolley
 Copyright 2002 (c) Milonic Solutions. All Rights Reserved.
 Plase vist http://www.milonic.co.uk/menu or e-mail menu3@milonic.com
 You may use this menu on your web site free of charge as long as you place prominent links to http://www.milonic.co.uk/menu and
 your inform us of your intentions with your URL AND ALL copyright notices remain in place in all files including your home page
 Comercial support contracts are available on request if you cannot comply with the above rules.
 This script featured on Dynamic Drive (http://www.dynamicdrive.com)
 */

//The following line is critical for menu operation, and MUST APPEAR ONLY ONCE. If you have more than one menu_array.js file rem out this line in subsequent files
menunum=0;menus=new Array();_d=document;function addmenu(){menunum++;menus[menunum]=menu;}function dumpmenus(){mt="<script language=javascript>";for(a=1;a<menus.length;a++){mt+=" menu"+a+"=menus["+a+"];"}mt+="<\/script>";_d.write(mt)}
//Please leave the above line intact. The above also needs to be enabled if it not already enabled unless this file is part of a multi pack.



////////////////////////////////////
// Editable properties START here //
////////////////////////////////////

// Special effect string for IE5.5 or above please visit http://www.milonic.co.uk/menu/filters_sample.php for more filters
if(navigator.appVersion.indexOf("MSIE 6.0")>0)
{
	effect = "Fade(duration=0.2);Alpha(style=0,opacity=88);Shadow(color='#777777', Direction=135, Strength=5)"
}
else
{
	effect = "Shadow(color='#777777', Direction=135, Strength=5)" // Stop IE5.5 bug when using more than one filter
}


timegap=500				// The time delay for menus to remain visible
followspeed=5			// Follow Scrolling speed
followrate=40			// Follow Scrolling Rate
suboffset_top=10;		// Sub menu offset Top position 
suboffset_left=10;		// Sub menu offset Left position

style1=[				// style1 is an array of properties. You can have as many property arrays as you need. This means that menus can have their own style.
"ffffff",					// Mouse Off Font Color"navy"
"0080ff",				// Mouse Off Background Color 
"ffffff",				// Mouse On Font Color
"909090",				// Mouse On Background Color
"000000",				// Menu Border Color 
12,						// Font Size in pixels
"normal",				// Font Style (italic or normal)
"bold",					// Font Weight (bold or normal)
"Verdana, Arial",		// Font Name
4,						// Menu Item Padding
"arrow.gif",			// Sub Menu Image (Leave this blank if not needed)
,						// 3D Border & Separator bar
"66ffff",				// 3D High Color 
"000099",				// 3D Low Color 
"Purple",				// Current Page Item Font Color (leave this blank to disable)
"pink",					// Current Page Item Background Color (leave this blank to disable)
"arrowdn.gif",			// Top Bar image (Leave this blank to disable)
"000000",				// Menu Header Font Color (Leave blank if headers are not needed)
"000099",				// Menu Header Background Color (Leave blank if headers are not needed) 
]



addmenu(menu=[		// This is the array that contains your menu properties and details
"mainmenu",			// Menu Name - This is needed in order for the menu to be called
5,					// Menu Top - The Top position of the menu in pixels
16,				// Menu Left - The Left position of the menu in pixels
,					// Menu Width - Menus width in pixels
1,					// Menu Border Width 
,					// Screen Position - here you can use "center;left;right;middle;top;bottom" or a combination of "center:middle"
style1,				// Properties Array - this is set higher up, as above
1,					// Always Visible - allows the menu item to be visible at all time (1=on/0=off)
"left",				// Alignment - sets the menu elements text alignment, values valid here are: left, right or center
effect,				// Filter - Text variable for setting transitional effects on menu activation - see above for more info
,					// Follow Scrolling - Tells the menu item to follow the user down the screen (visible at all times) (1=on/0=off)
1, 					// Horizontal Menu - Tells the menu to become horizontal instead of top to bottom style (1=on/0=off)
,					// Keep Alive - Keeps the menu visible until the user moves over another menu or clicks elsewhere on the page (1=on/0=off)
,					// Position of TOP sub image left:center:right
,					// Set the Overall Width of Horizontal Menu to 100% and height to the specified amount (Leave blank to disable)
,					// Right To Left - Used in Hebrew for example. (1=on/0=off)
,					// Open the Menus OnClick - leave blank for OnMouseover (1=on/0=off)
,					// ID of the div you want to hide on MouseOver (useful for hiding form elements)
,					// Reserved for future use
,					// Reserved for future use
,					// Reserved for future use
,"PICmicros&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp;&nbsp;","show-menu=pics",,"",1
,"SCENIX&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp;&nbsp;","show-menu=scenix",,"",1
,"GameBoy&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp;&nbsp;","show-menu=gameboy",,"",1
,"PCB's&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp;&nbsp;","show-menu=pcb",,"",1
,"DSP&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp;&nbsp;","show-menu=dsp",,"",1
])

	addmenu(menu=["pics",,,190,1,,style1,0,"left",effect,0,,,,,,,,,,,
	,"PPWIN programmer","Pics/PICmain.htm",,,1
	,"uJDM  programmer","uJDM/uJDMmain.htm",,,1
	,"PP875 programmer","Pics/pp875/PP875main.htm",,,1
	,"PICcom development tool","PICcom/PICcom.htm",,,1
	])

	addmenu(menu=["scenix",,,150,1,,style1,0,"left",effect,0,,,,,,,,,,,
	,"Fluffy2 programmer","Sx/SXmain.htm",,,1
	,"Fluffy programmer","Sx/fluffy/SXmain.htm",,,1
	])

	addmenu(menu=["gameboy",,,140,1,,style1,0,"left",effect,0,,,,,,,,,,,
	,"GameBoy Advance","Gba/GBAmain.html",,,1
	,"GameBoy","Gameboy/Gbmain.htm",,,1
	])

	addmenu(menu=["pcb",,,150,1,,style1,0,"left",effect,0,,,,,,,,,,,
	,"How to make PCB's","PCB/PCB.html",,,1
	])

	addmenu(menu=["dsp",,,100,1,,style1,0,"left",effect,0,,,,,,,,,,,
	,"Audio DSP's","Dsp/DSPmain.htm",,,1
	])

dumpmenus()