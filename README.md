Open Market
===========


Introduction
============

This project is a client-server program that emulates an open market.
A user that launches the program can use one of the defined actions to interact with the market.

The market is represented by a shared memory. Each item in the market is called a stock. 
A stock has a name, which is the name of the product. It also has a price, and the quantiy of items in it. Finally, each stock has a "producer" field which is the name/pseudo of the person this stock belongs to.


Each person can offer products for sale, can modify their own stocks, and can buy items from others. These actions will be explained in the following section.


The following commands define the actions that each user can take:
==================================================================

1) add: used to add a new stock
"add 5 apples 2.3" means that the user added a stock of apples, that contains 5 elements, priced at 2.3 euros/piece.

2) addTo: used to add a number of items to an existing stock. This stock must exist for the user who is using this command. Each user can only add to his own stock. 
"addTo apples 3" adds 3 items to the stock of apples.

3)removeFrom: used to remove a number of items from an existing stock. This stock must exist for the user who is using this command. Each user can only remove from his own stock.
"removeFrom apples 3" will remove 3 items from the stock of apples. 

4)modifyPrice changes the price of the specified stock
"modifyPrice apples 3" will change the price of the apple stock to 3. This stock must exist for the person who is executing this command.

5)removeStock: used to completely remove an existing stock that belongs to the person executing it.
"removeStock apple" will remove the stock of apples from the market

6)buy: this command is used to buy an item. Buying is decreasing the stock of this item by the amount specified.
"buy 3 apples from Toto" will remove 3 apples from Toto's stock if this stock exists for Toto and contains more than 3 apples.

7)display: this command will list the full content of the market/shared memory. Use it simple as "display" without parameters. 

8)quit: this command will deletes the entirety of a person's stock from the market and closes the client program from which it was called. Use it as it is without parameters: "quit".

9)help: typing "help" into the program will show a list of commands with ane explanation.


How to run the program
=======================

1) Open a terminal and run ./server
2) Open a second terminal and run ./client [ip address] 31000
3) You can open as many terminals as you want and run clients
4) To quit,use the "quit" command
5) To close the server, use ctrl+c
