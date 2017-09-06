

import socket

def readIntFromSocket(conn):
    count = 0
    result = 0
    while count < 4:
        data = conn.recv(1)
    
        if len(data) == 0:
            return 0
    
        print("Data {} - {}").format(data, ord(data[0]))
        result += ord(data[0])
        print("Result {}").format(result)
        count += 1
       
    return result

def listen():
    connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connection.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    connection.bind(('0.0.0.0', 9999))
    connection.listen(10)
    while True:
        current_connection, address = connection.accept()
        print("connected")
        while True:
            len = readIntFromSocket(current_connection)
            if len == 0:
                print("Connection Closed\n")
                current_connection.shutdown(1)
                current_connection.close()
                break
            
            print("len {} ").format(len)
            
                
            msgType = readIntFromSocket(current_connection)
             
            data = current_connection.recv(2048)

            print("data {} ").format(data)
            
            if data == 'quit\r\n':
                current_connection.shutdown(1)
                current_connection.close()
                break

            elif data == 'stop\r\n':
                current_connection.shutdown(1)
                current_connection.close()
                exit()

            elif data:
                current_connection.send(data)
                print data


if __name__ == "__main__":
    try:
        listen()
    except KeyboardInterrupt:
        pass