Concerning the libTCOD library:

/*
* Copyright (c) 2010 Jice
* All rights reserved.
*

* Redistribution and use in source and binary forms, with or without

* modification, are permitted provided that the following conditions are met:

*     * Redistributions of source code must retain the above copyright

*       notice, this list of conditions and the following disclaimer.

*     * Redistributions in binary form must reproduce the above copyright

*       notice, this list of conditions and the following disclaimer in the

*       documentation and/or other materials provided with the distribution.

*     * The name of Jice may not be used to endorse or promote products

*       derived from this software without specific prior written permission.

*
* THIS SOFTWARE IS PROVIDED BY JICE ``AS IS'' AND ANY

* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED

* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE

* DISCLAIMED. IN NO EVENT SHALL JICE BE LIABLE FOR ANY

* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES

* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;

* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND

* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT

* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS

* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




To play the game in its current state:

Movement maybe achieved through the use of the arrow keys. Additionally, the numpad can be used for movement. 8 is up, 6 is right, 2 is down, and 4 is right. 7, 9, 3, and 1 are used for the respective diagonals, and 5 is used to wait for a turn, taking no movement. Again, these movements are mapped to the number pad, not to the number keys at the top of the keyboard.
	
To attack monsters, simply attempt to move into the square they occupy. You will attack them and, most likely, they will strike you back. Defeating monsters awards experience, and experience allows you to level up. Upon leveling, you may choose an attribute to increase by moving through the menu with the arrow keys and using Enter to make a selection.
	
To pick up various items on the floor, press 'g' while standing on the item. This will add it to your inventory.

To access the inventory, press 'i.' This will bring up a menu full of the items in your inventory, one of which you can choose to use by pressing the keyboard character indicated next to the item in question in parentheses. 
	
Pressing the 'l' key allows one to look at a tile and see what is on it. Once 'l' is pressed, a small cursor appears on the currently selected tile. This can be moved using the arrow keys, and once the desired tile is highlighted, pressing Enter will reveal relevant tile information in the message area. Similarly, certain ranged abilities will require the selection of a target tile. Selecting a tile is an identical process to selecting a tile for the look command (movement of cursor with arrow keys, selection with Enter). If the	ranged ability affects an area/radius of tiles, the cursor will take the form of said radius, centered on the currently selected tile. 
	
On each floor of the dungeon is a tile with the '>' character. This tile indicates that stairs to a lower dungeon level are present on that tile. Standing on this tile and pressing '>' (Shift + '.') will allow one to travel to a lower dungeon level. Beware, travelling too deep into the dungeon may spell disaster for a fledgling adventurer! 

Screenshots may be taken using the printscreen key. They will be named "screenshotNNN.png", where NNN indicates the lowest available number. The first screenshot will be named "screenshot000.png" and the next named "screenshot001.png" and so on. 

The player may find some monsters or machines on the screen which are neutral towards him; as such, he may be less inclined to attack them. To attack any character on the screen, press '=' to enter a "hostile stance," which will allow you to attack any character on the screen that is capable of being destroyed.

Some of these neutral characters (discussed above) are in fact vending machines. Obviously, the player may purchase items from them should he have sufficient PetaBitcoins. In the future, the vending machines will have a limited amount of material from which they can create objects (they are essentially fancy 3d-printers), and the player will have to provide them with spools or cartridges of more material, should he wish to buy something from that particular machine. Alternatively, he could destroy the machine and collect a spool/cartidge of material to feed into a machine encountered later on.
