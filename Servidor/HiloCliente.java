package Servidor;
import java.net.*;
import java.io.*;
import java.nio.charset.StandardCharsets;

public class HiloCliente extends Thread implements Observadorsito {

    private Socket stream = null;
    private BufferedReader entrada = null;
    private PrintWriter salida = null;

    private server miServidor;
    private Partida miPartida;
    private jugador miJugador;

    public HiloCliente(Socket conexion, server servidor) {
        this.stream = conexion;
        this.miServidor = servidor;
        this.miPartida = null;
        this.miJugador = null;
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

            String solicitudRol = entrada.readLine();
            boolean esEspectador = solicitudRol != null && solicitudRol.trim().equalsIgnoreCase("ROL,ESPECTADOR");
            boolean esJugador = !esEspectador;

            if (esJugador) {
                miPartida = miServidor.crearPartidaJugador();
                if (miServidor.esModoCooperativo()) {
                    miJugador = miPartida.asignarJugador(2, 1);
                } else {
                    miJugador = miPartida.asignarJugador();
                }
            } else {
                miPartida = miServidor.obtenerPartidaActiva();
            }

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
                        if (esJugador && partesMensaje.length > 2) {
                            try {
                                int idAlien = Integer.parseInt(partesMensaje[1]);
                                miPartida.alienEliminado(miJugador, idAlien, partesMensaje[2]);
                            } catch (NumberFormatException errorAlien) {
                                System.out.println("Alien invalido: " + errorAlien.getMessage());
                            }
                        }
                        break;

                    case "MATE_OVNI":
                        if (esJugador && partesMensaje.length > 1) {
                            try {
                                int idOvni = Integer.parseInt(partesMensaje[1]);
                                miPartida.ovniEliminado(miJugador, idOvni);
                            } catch (NumberFormatException errorOvni) {
                                System.out.println("OVNI invalido: " + errorOvni.getMessage());
                            }
                        }
                        break;

                    case "BUNKER_HIT":
                        if (partesMensaje.length > 2) {
                            try {
                                int indiceBunker = Integer.parseInt(partesMensaje[1]);
                                int dano = Integer.parseInt(partesMensaje[2]);
                                miPartida.bunkerGolpeado(indiceBunker, dano);
                            } catch (NumberFormatException errorBunker) {
                                System.out.println("Bunker invalido: " + errorBunker.getMessage());
                            }
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
            if (miJugador != null) {
                miPartida.liberarJugador(miJugador);
            }
            stream.close();
        } catch (IOException errorsito) {
            System.out.println("Despiche general en el hilo: " + errorsito.getMessage());
            if (miJugador != null) {
                miPartida.liberarJugador(miJugador);
            }
        }
    }
}
