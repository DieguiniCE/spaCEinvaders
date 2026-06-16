package Servidor;
import java.io.*;
import java.net.*;

public class Cliente {

    private Socket stream = null;
    private BufferedReader entrada = null; //no se usa DataInputStream xq aparece que está deprecated que no lee bien las varas hay que ponerle el lector del buffer
    private DataOutputStream salida = null;

    public Cliente(String ip, int puerto){

        //esto es para conectarse
        try {
            stream = new Socket(ip, puerto);
            System.out.println("Conectado"); //acá se crea el socket para conectarse

            entrada = new BufferedReader(new InputStreamReader(System.in)); //agarra la entrada de la terminal

            salida = new DataOutputStream(System.out); //manda la salida por el socket     
        } 
        catch (UnknownHostException error) { //acá agarra los errores raros
            System.out.println(error);
            return;
        }
        catch (IOException errorsito){ //acá agarra los errores de entrada y salida
            System.out.println(errorsito);
            return;
        }

        String mensaje = "Hola server"; //hacemos la variable mensaje

        while (!mensaje.equals("Over")){
            try {
                mensaje = entrada.readLine();
                salida.writeUTF(mensaje);
            } catch (IOException errorsito2) {
                System.out.println(errorsito2);
            }
        }

        //esto es para desconectarse
        try {
            entrada.close();
            salida.close();
            stream.close();
        } 
        catch (IOException errorsito2) {
                System.out.println(errorsito2);
        }
        }
        


    public static  void main(String[] args) {
        Cliente c = new Cliente("0.0.0.0", 5000);
    }
}
    
    