These are test cases to test multithreaded chat room application.

Every message will show on the server side.


test case 1:
Purpose:test if these chat server can accept multiple users. 

implement: Open a terminal and run server program. At the same time, open other two terminals as clients and run client program.

output: when client connect to the server successfully, the server will send welcome and history messages to the user and the user can start to chat with people.
On the other hand, when the server receive a new connection, the new connection notification will be printed on the server terminal.

test case 2:
Purpose: test when the server not ready, the client side can wait for the server and try to connect in every 3s.

implement: Open a terminal and run client program first and then open another terminal and run server program.

output: when the client program run first without a server start, the client side terminal will show error connect and wait for 3s and try to connect again. When the server start, the client side will connect to the server successfully.

test case 3:
Purpose:test if user can see history messages

implement: one client connect to the chat server and type some word in the chat room. Then another user connect to the chat room.

output: the second user will see the history messages after the welcome and usage instruction.


test case 4:
Purpose:test @who comment

implement: allow three clients connect to the server first and one of the user type @who.

output: the user will see all the current users status.

test case 5:
Purpose:test @name comment, this comment is designed to change the username and at the same time, the server should check if the name already in use.

implement:allow three clients, called jenny, Lin, Manying respectively, connect to the server first and Manying try to change to name to jenny, so she types @name jenny.

output: the server will send her a message to tell her the username is already in use.

implement: Manying then types @name Kat

output: the server will announce Manying change username notification to everyone.

test case 6:
Purpose:test @private comment, user can send private messages to multiple users.

implement: allow four clients,called jenny, Lin, Manying, John respectively, connect to the server and jenny types @private Manying, then types @private Lin, and starts to type messages

output:the messages that jenny sent are only seen by Lin and Manying

test case 7:
Purpose: test when one user is already in private target, he/she can not accept another private messages.

implement: continue the scenario above, John types @private Lin.

output: the server will send message to tell John that Lin is already in private target and the request is denied.

test case 8:
Purpose: test @end comment. When a user is in private mode, he/she can type @end <name> to disconnect the private status with the person. when he/she doesn’t connect to any private user, he/she returns to public mode

implement: continue the scenario above, Jenny types @end Lin, and types some messages.

output:the messages that Jenny sent can only be seen by Manying

implement: Jenny types @end Manying. 

output: the messages that jenny sent is seen by everyone

test case 9:
Purpose: test @exit comment

implement: continue the scenario above, jenny types @exit

output: the server will send announce to the everyone about one user left. jenny client side will exit the client program.

test case 10:
Purpose: test when the server shutdown, it should output public messages log file and the file should locate under the application folder and be named by date and time. 

implement: continue the scenario above, and now, we try to use control-c to shutdown the server.

output:client side will receive message about server disconnection. Check the folder, there will be a log file named by date and time.





 




  
