package Servidor;
import java.net.*;
import java.io.*;

public class server {

    private ServerSocket streamServidor = null;

    public server(int port) {
        // Acá se inicia el server
        try {
            streamServidor = new ServerSocket(port);
            System.out.println("Servidor iniciado puramente. Esperando clientes...");

            Partida partidaPrincipal = new Partida(); // Creamos la partida general
            int cantidadJugadores = 0; // Para llevar la cuenta de cuántos juegan

            // Ciclo infinito para escuchar a todo el que llegue
            while (true) {
                Socket stream = streamServidor.accept();
                System.out.println("¡Cayó alguien nuevo al server!");

                boolean esJugador = false;
                jugador jugadorAsignado = null;
                
                // Si hay menos de 2, lo metemos a jugar
                if (cantidadJugadores < 2) {
                    cantidadJugadores++;
                    esJugador = true;

                    // Se crea con X=0, Y=0, 3 vidas, y su ID de jugador es 1 o 2
                    jugadorAsignado = new jugador(0, 0, 3, cantidadJugadores);
                    partidaPrincipal.agregarJugador(jugadorAsignado);
                    System.out.println("Se le asignó el control: JUGADOR " + cantidadJugadores);

                } else {
                    // Si ya están los 2, se meten de sapos
                    System.out.println("Se le asignó el rol: SAPO ");
                }

                // Le hacemos su propio hilo para no pegar el server
                HiloCliente nuevoCliente = new HiloCliente(stream, esJugador, partidaPrincipal, jugadorAsignado);
                nuevoCliente.start(); 
            }
        }
        catch (IOException errorsito) { // Acá agarra los despiches de red
            System.out.println("Murió el servidor: " + errorsito.getMessage());
            return;
        }
    }

    public static void main(String[] args) {
        server c = new server(5000);
    }
}