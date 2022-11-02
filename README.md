# SenderReceiverChibiOS

Se propone un sistema de Sender-Receiver con 2 arduinos mkr-wan1310 conectados en serial y a uno de ellos se conecta un sensor ultrasonido.

La idea es que uno lee los datos del sensor ultrasonido, se lo manda a la otra placa, lo muestra,
esta reenvía confirmación de que lo recibió y además en esta se permiten introducir una serie de opciones.
Se propone usar help para que muestre toda la lista de comandos disponibles.
