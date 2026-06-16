package Servidor;
import java.net.*;
import java.io.*;

public class server {

    private ServerSocket streamServidor = null;

    public server(int port) {
        //esto es para conectarse
        try
        {
            streamServidor = new ServerSocket(port);
            System.out.println("Conectado");
            System.out.println("Esperando conexiones");

            //hacemos un ciclo infinito para que el servidor siempre escuche
            while (true) 
            {
                //el servidor se queda esperando acá hasta que alguien se conecte
                Socket stream = streamServidor.accept();
                System.out.println("Cliente aceptado");

                //cuando alguien se conecta, creamos un hilo para ese cliente
                //y le pasamos el socket (stream) que acaba de conectarse
                HiloCliente nuevoCliente = new HiloCliente(stream);
                
                //iniciamos el hilo para que trabaje en paralelo
                nuevoCliente.start(); 
            }
		}
        catch (IOException errorsito){ //acá agarra los errores de entrada y salida
				System.out.println(errorsito);
				return;
		}
    }

	public static void main(String[] args) {
        server c = new server(5000);
    }
}