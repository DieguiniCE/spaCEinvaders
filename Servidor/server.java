package Servidor;
import java.net.*;
import java.io.*;

public class server {

    private static final int ANCHO_PANTALLA = 800;
    private static final int ALTO_PANTALLA = 600;
    private static final int ANCHO_CANON = 40;
    private static final int VELOCIDAD_CANON = 5;

    private Socket stream = null;
    private ServerSocket streamServidor = null;
    private InputStream entrada = null;
    private OutputStream salida = null;
    private jugador jugador1;

    public server(int port) {
        jugador1 = new jugador(ANCHO_PANTALLA / 2, ALTO_PANTALLA - 50, 3);

        try
        {
            streamServidor = new ServerSocket(port);
            System.out.println("Servidor spaCEinvaders escuchando en puerto " + port);

            System.out.println("Esperando conexion");

            stream = streamServidor.accept();
            System.out.println("Cliente aceptado");

            entrada = stream.getInputStream();
            salida = stream.getOutputStream();

            enviarEstado();
        }

        catch (UnknownHostException error) {
            System.out.println(error);
            return;
        }
        catch (IOException errorsito){
            System.out.println(errorsito);
            return;
        }

        int comando;

        while (true)
        {
            try
            {
                comando = entrada.read();
                if (comando == -1) {
                    break;
                }

                procesarComando((char) comando);
                enviarEstado();
            }
            catch (IOException errorsito2)
            {
                System.out.println(errorsito2);
                break;
            }
        }

        System.out.println("Cerrando conexion");

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

    private void procesarComando(char comando) {
        switch (comando) {
            case 'L':
                jugador1.desplazamiento(-VELOCIDAD_CANON, 0);
                System.out.println("Comando: izquierda");
                break;
            case 'R':
                jugador1.desplazamiento(VELOCIDAD_CANON, 0);
                System.out.println("Comando: derecha");
                break;
            case 'F':
                jugador1.disparo();
                System.out.println("Comando: disparo");
                break;
            default:
                System.out.println("Comando desconocido: " + comando);
                break;
        }

        limitarPosicionJugador();
    }

    private void limitarPosicionJugador() {
        int x = jugador1.getposX();
        int y = jugador1.getposY();

        if (x < 0) {
            x = 0;
        }
        if (x > ANCHO_PANTALLA - ANCHO_CANON) {
            x = ANCHO_PANTALLA - ANCHO_CANON;
        }
        if (y < 0) {
            y = 0;
        }
        if (y > ALTO_PANTALLA - 20) {
            y = ALTO_PANTALLA - 20;
        }

        jugador1.establecerPosicion(x, y);
    }

    private void enviarEstado() {
        try {
            String mensaje = String.format(
                "STATE:%d,%d,%d,%d\n",
                jugador1.getposX(),
                jugador1.getposY(),
                jugador1.getVidas(),
                jugador1.getPuntos()
            );
            salida.write(mensaje.getBytes("UTF-8"));
            salida.flush();
        } catch (IOException error) {
            System.out.println(error);
        }
    }

    public static void main(String[] args) {
        int puerto = 8080;
        if (args.length > 0) {
            puerto = Integer.parseInt(args[0]);
        }
        server c = new server(puerto);
    }
}
