package Servidor;
import java.net.*;
import java.io.*;
import java.nio.charset.StandardCharsets;

public class HiloCliente extends Thread implements Observadorsito {

    private Socket stream = null;
    private BufferedReader entrada = null;
    private PrintWriter salida = null;

    private boolean esJugador;
    private Partida miPartida;
    private jugador miJugador;

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
                salida.println(estadoJuego);
            }
        } catch (Exception errorsito3) {
            System.out.println("Error mandando info al cliente: " + errorsito3.getMessage());
        }
    }

    public void run() {
        try {
            entrada = new BufferedReader(new InputStreamReader(
                stream.getInputStream(), StandardCharsets.UTF_8));
            salida = new PrintWriter(new OutputStreamWriter(
                stream.getOutputStream(), StandardCharsets.UTF_8), true);

            miPartida.registrarCliente(this, esJugador, miJugador);

            String mensaje;
            while ((mensaje = entrada.readLine()) != null && !mensaje.equals("Over")) {
                System.out.println("Llego mensaje: " + mensaje);

                String[] partesMensaje = mensaje.split(",");
                String comando = partesMensaje[0];

                switch (comando) {
                    case "MOVER":
                        if (esJugador && partesMensaje.length > 1) {
                            miPartida.moverJugador(miJugador, partesMensaje[1]);
                        }
                        break;

                    case "DISPARO":
                        if (esJugador) {
                            miPartida.jugadorDispara(miJugador);
                        }
                        break;

                    case "MATE_ALIEN":
                        if (esJugador && partesMensaje.length > 1) {
                            miPartida.alienEliminado(miJugador, partesMensaje[1]);
                        }
                        break;

                    case "PERDI_VIDA":
                        if (esJugador) {
                            miPartida.jugadorPierdeVida(miJugador);
                        }
                        break;

                    case "LIMPIE_PANTALLA":
                        if (esJugador) {
                            miPartida.oleadaLimpiada(miJugador);
                        }
                        break;

                    case "Crear":
                        if (partesMensaje.length >= 4) {
                            miPartida.adminCrearAlien(partesMensaje[1], partesMensaje[2], partesMensaje[3]);
                        }
                        break;

                    case "OVNI":
                        if (partesMensaje.length >= 3) {
                            miPartida.adminCrearOvni(partesMensaje[1], partesMensaje[2]);
                        }
                        break;

                    case "Velocidad":
                        if (partesMensaje.length >= 2) {
                            miPartida.adminVelocidad(partesMensaje[1]);
                        }
                        break;

                    case "Bunkers":
                        if (partesMensaje.length >= 2) {
                            miPartida.adminBunkers(partesMensaje[1]);
                        }
                        break;

                    default:
                        System.out.println("Comando desconocido: " + comando);
                        break;
                }
            }

            miPartida.quitarObservador(this);
            stream.close();
        } catch (IOException errorsito) {
            System.out.println("Despiche general en el hilo: " + errorsito.getMessage());
        }
    }
}
