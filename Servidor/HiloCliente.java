package Servidor;
import java.net.*;
import java.io.*;

public class HiloCliente extends Thread implements Observadorsito {
    
    private Socket stream = null;
    private DataInputStream entrada = null;
    private DataOutputStream salida = null;
    
    private boolean esJugador;
    private Partida miPartida;
    private jugador miJugador; // Guarda qué jugador es el dueño del cliente

    public HiloCliente(Socket conexion, boolean esJugador, Partida partida, jugador miJugador) {
        this.stream = conexion;
        this.esJugador = esJugador;
        this.miPartida = partida;
        this.miJugador = miJugador;
    }

    @Override
    public void recibirActualizacion(String estadoJuego) {
        try {
            if (salida != null) {
                salida.writeUTF(estadoJuego);
                salida.flush(); //lo borra
            }
        } catch (IOException errorsito3) {
            System.out.println("Error mandando info al cliente: " + errorsito3.getMessage());
        }
    }

   
    public void run() {
        try {

            // dartos de entrada y salida
            entrada = new DataInputStream(new BufferedInputStream(stream.getInputStream()));
            salida = new DataOutputStream(stream.getOutputStream());

            miPartida.agregarEspectador(this);

            String mensaje = ""; 
            
            // Ciclo hasta que manden "Over" 
            while (!mensaje.equals("Over")) {
                try {
                    mensaje = entrada.readUTF();
                    System.out.println("Llegó mensaje: " + mensaje);
                    
                    // se parte el mensaje por comas
                    String[] partesMensaje = mensaje.split(",");
                    String comando = partesMensaje[0];

                    switch (comando) {
                        // comunicación
                        case "MATE_ALIEN":
                            if (esJugador && partesMensaje.length > 1) {
                                miPartida.alienEliminado(miJugador, partesMensaje[1]);
                            }
                            break;
                            
                        case "PERDI_VIDA":
                            if (esJugador) miPartida.jugadorPierdeVida(miJugador);
                            break;

                        case "LIMPIE_PANTALLA":
                            if (esJugador) miPartida.oleadaLimpiada(miJugador);
                            break;

                        
                        case "Crear":
                            // Ej: Crear,1,1,1000
                            if (partesMensaje.length >= 4) miPartida.adminCrearAlien(partesMensaje[1], partesMensaje[2], partesMensaje[3]);
                            break;

                        case "OVNI":
                            // Ej: OVNI,I-D,1500
                            if (partesMensaje.length >= 3) miPartida.adminCrearOvni(partesMensaje[1], partesMensaje[2]);
                            break;

                        case "Velocidad":
                            // Ej: Velocidad,100
                            if (partesMensaje.length >= 2) miPartida.adminVelocidad(partesMensaje[1]);
                            break;

                        case "Bunkers":
                            // Ej: Bunkers,70%
                            if (partesMensaje.length >= 2) miPartida.adminBunkers(partesMensaje[1]);
                            break;

                        default:
                            System.out.println("Ese comando no existe papi: " + comando);
                            break;
                    }
                }
                catch (IOException errorsito2) {
                    System.out.println("Un mae se desconectó.");
                    break; 
                }
            }
            
            // Lo borramos de la lista si se sale
            miPartida.quitarEspectador(this);
            stream.close();
        } 
        catch (IOException errorsito) { 
            System.out.println("Despiche general en el hilo: " + errorsito.getMessage());
        }
    }
}