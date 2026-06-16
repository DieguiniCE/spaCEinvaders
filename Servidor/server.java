package Servidor;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

public class server {

    private static final int ANCHO_PANTALLA = 800;
    private static final int ALTO_PANTALLA = 600;

    private final Partida partida;
    private final ServerSocket streamServidor;
    private int jugadoresAsignados;

    public server(int port) {
        this.partida = new Partida();
        this.jugadoresAsignados = 0;

        try {
            this.streamServidor = new ServerSocket(port);
            System.out.println("Servidor spaCEinvaders escuchando en puerto " + port);
            pedirConfiguracionInicial();
            this.partida.inicializarOleadaDefecto();

            Thread consolaAdmin = new Thread(this::leerComandosAdmin, "AdminConsole");
            consolaAdmin.setDaemon(true);
            consolaAdmin.start();

            aceptarClientes();
        } catch (IOException error) {
            System.out.println(error);
            throw new RuntimeException(error);
        }
    }

    private void pedirConfiguracionInicial() throws IOException {
        BufferedReader entrada = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8));
        System.out.print("Velocidad inicial de aliens (Enter = 1.0): ");
        String linea = entrada.readLine();

        if (linea != null) {
            String velocidad = linea.trim();
            if (!velocidad.isEmpty()) {
                partida.adminVelocidad(velocidad);
            }
        }

        System.out.println("Comandos en caliente: Crear X Y Pts | OVNI R 100 | Velocidad 100 | Bunkers 75");
    }

    private void aceptarClientes() {
        while (true) {
            try {
                Socket stream = streamServidor.accept();
                boolean esJugador = jugadoresAsignados < 2;
                jugador jugadorAsignado = null;

                if (esJugador) {
                    jugadoresAsignados += 1;
                    jugadorAsignado = partida.crearJugador(jugadoresAsignados);
                    partida.agregarJugador(jugadorAsignado);
                    System.out.println("Jugador " + jugadoresAsignados + " conectado desde " + stream.getRemoteSocketAddress());
                } else {
                    System.out.println("Espectador conectado desde " + stream.getRemoteSocketAddress());
                }

                HiloCliente hiloCliente = new HiloCliente(stream, esJugador, partida, jugadorAsignado);
                hiloCliente.start();
            } catch (IOException errorConexion) {
                System.out.println("Error aceptando cliente: " + errorConexion.getMessage());
                break;
            }
        }
    }

    private void leerComandosAdmin() {
        BufferedReader entrada = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8));

        while (true) {
            try {
                String linea = entrada.readLine();
                if (linea == null) {
                    break;
                }

                String comando = linea.trim();
                if (comando.isEmpty()) {
                    continue;
                }

                String[] partes = comando.split("\\s+");
                switch (partes[0].toLowerCase()) {
                    case "crear":
                        if (partes.length >= 4) {
                            partida.adminCrearAlien(partes[1], partes[2], partes[3]);
                        } else {
                            System.out.println("Uso: Crear X Y Pts");
                        }
                        break;
                    case "ovni":
                        if (partes.length >= 3) {
                            partida.adminCrearOvni(partes[1], partes[2]);
                        } else {
                            System.out.println("Uso: OVNI direccion puntos");
                        }
                        break;
                    case "velocidad":
                        if (partes.length >= 2) {
                            partida.adminVelocidad(partes[1]);
                        } else {
                            System.out.println("Uso: Velocidad valor");
                        }
                        break;
                    case "bunkers":
                        if (partes.length >= 2) {
                            partida.adminBunkers(partes[1]);
                        } else {
                            System.out.println("Uso: Bunkers porcentaje");
                        }
                        break;
                    default:
                        System.out.println("Comandos: Crear, OVNI, Velocidad, Bunkers");
                        break;
                }
            } catch (IOException errorAdmin) {
                System.out.println("Error leyendo admin: " + errorAdmin.getMessage());
                break;
            }
        }
    }

    public static void main(String[] args) {
        int puerto = 5000;
        if (args.length > 0) {
            puerto = Integer.parseInt(args[0]);
        }
        new server(puerto);
    }
}
