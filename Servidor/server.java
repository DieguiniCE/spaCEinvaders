package Servidor;
import java.net.*;
import java.io.*;

public class server {

    private Socket stream = null;
    private ServerSocket streamServidor = null;
    private DataInputStream entrada = null;
	private DataOutputStream salida = null;

    public server(int port) {
        //esto es para conectarse
        try
        {
            streamServidor = new ServerSocket(port);
            System.out.println("Conectado");

            System.out.println("Waiting for a client ...");

            stream = streamServidor.accept();
            System.out.println("Client accepted");

            // Takes input from the client socket
            entrada = new DataInputStream(new BufferedInputStream(stream.getInputStream()));
            salida = new DataOutputStream(stream.getOutputStream());
		}

		catch (UnknownHostException error) { //acá agarra los errores raros
				System.out.println(error);
				return;
        }
        catch (IOException errorsito){ //acá agarra los errores de entrada y salida
				System.out.println(errorsito);
				return;
		}

        String mensaje = ""; //hacemos la variable mensaje

        while (!mensaje.equals("Over"))
        {
            try
            {
                mensaje = entrada.readUTF();
					salida.writeUTF(mensaje);
                System.out.println(mensaje);
				}
            catch (IOException errorsito2)
            {
				System.out.println(errorsito2);
            }
			}

        System.out.println("Closing connection");

        // Close connection
        try
        {
            stream.close();
            streamServidor.close();
        }
        catch(IOException error)
        {
            System.out.println(error);
        }
    }
}