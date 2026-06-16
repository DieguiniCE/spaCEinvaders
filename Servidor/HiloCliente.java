package Servidor;
import java.net.*;
import java.io.*;

//extender de Thread es lo que permite que esto corra en paralelo
public class HiloCliente extends Thread {
    
    private Socket stream = null;
    private DataInputStream entrada = null;
	private DataOutputStream salida = null;
    
    //el constructor recibe el socket del cliente que le mandó el servidor
    public HiloCliente(Socket conexion) {
        this.stream = conexion;
    }

    //todo lo que esté adentro de run es lo que va a hacer el hilo de forma independiente
    public void run() {
        try {
            // agarra lo quue le mande el cliente
            entrada = new DataInputStream(new BufferedInputStream(stream.getInputStream()));
            salida = new DataOutputStream(stream.getOutputStream());

            String mensaje = ""; //hacemos la variable mensaje

            while (!mensaje.equals("Over"))
            {
                try
                {
                    mensaje = entrada.readUTF();
                    salida.writeUTF(mensaje); //devuelve el mensaje 
                    System.out.println("Mensaje recibido en hilo: " + mensaje);
                }
                catch (IOException errorsito2)
                {
                    System.out.println(errorsito2);
                    break; //salimos del ciclo si se cae la conexion
                }
            }

            System.out.println("Cerrando conexion de un cliente");

            // Cerrar conexión específica de este cliente
            stream.close();
        } 
        catch (IOException errorsito) { //acá agarra los errores de entrada y salida
            System.out.println(errorsito);
        }
    }
}