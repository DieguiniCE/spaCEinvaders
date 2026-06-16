package Servidor;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;

public class server {

    private static final int ANCHO_PANTALLA = 800;
    private static final int ALTO_PANTALLA = 600;

    private final ServerSocket streamServidor;
    private final ArrayList<Partida> partidasActivas;
    private int contadorPartidas;
    private double velocidadInicial;
    private boolean modoCooperativo;
    private Partida partidaCooperativa;

    public server(int port) {
        this.partidasActivas = new ArrayList<>();
        this.contadorPartidas = 0;
        this.velocidadInicial = 1.0;
        this.modoCooperativo = true;
        this.partidaCooperativa = null;

        try {
            this.streamServidor = new ServerSocket(port);
            System.out.println("Servidor spaCEinvaders escuchando en puerto " + port);
            pedirModoJuego();
            pedirConfiguracionInicial();
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
                try {
                    velocidadInicial = Double.parseDouble(velocidad);
                } catch (NumberFormatException error) {
                    velocidadInicial = 1.0;
                }
            }
        }

        System.out.println("Configuracion de sesiones aplicada segun el modo elegido.");
        System.out.println("Comandos en caliente: Crear X Y Pts | OVNI R 100 | Velocidad 100 | Bunkers 75");
    }

    private void pedirModoJuego() throws IOException {
        BufferedReader entrada = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8));
        System.out.print("Modo de juego (C = cooperativo, S = solitario) [C]: ");
        String linea = entrada.readLine();

        String modo = (linea == null) ? "" : linea.trim();
        modoCooperativo = !modo.equalsIgnoreCase("S");

        if (modoCooperativo) {
            partidaCooperativa = new Partida(1);
            partidaCooperativa.adminVelocidad(Double.toString(velocidadInicial));
            partidaCooperativa.inicializarOleadaDefecto();
            partidasActivas.add(partidaCooperativa);
            System.out.println("Modo cooperativo activo: J1 y J2 comparten la misma partida.");
        } else {
            partidaCooperativa = null;
            System.out.println("Modo solitario activo: cada jugador tendrá su propia partida.");
        }
    }

    public synchronized Partida crearPartidaJugador() {
        if (modoCooperativo) {
            return partidaCooperativa;
        }

        Partida partidaNueva = new Partida(++contadorPartidas);
        partidaNueva.adminVelocidad(Double.toString(velocidadInicial));
        partidaNueva.inicializarOleadaDefecto();
        partidasActivas.add(partidaNueva);
        return partidaNueva;
    }

    public synchronized Partida obtenerPartidaActiva() {
        if (modoCooperativo) {
            return partidaCooperativa;
        }

        if (partidasActivas.isEmpty()) {
            return crearPartidaJugador();
        }

        return partidasActivas.get(partidasActivas.size() - 1);
    }

    public boolean esModoCooperativo() {
        return modoCooperativo;
    }

    public synchronized ArrayList<Partida> obtenerSesiones() {
        return new ArrayList<>(partidasActivas);
    }

    private void aceptarClientes() {
        while (true) {
            try {
                Socket stream = streamServidor.accept();
                System.out.println("Cliente conectado desde " + stream.getRemoteSocketAddress());

                HiloCliente hiloCliente = new HiloCliente(stream, this);
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
                            for (Partida partidaSesion : obtenerSesiones()) {
                                partidaSesion.adminCrearAlien(partes[1], partes[2], partes[3]);
                            }
                        } else {
                            System.out.println("Uso: Crear X Y Pts");
                        }
                        break;
                    case "ovni":
                        if (partes.length >= 3) {
                            for (Partida partidaSesion : obtenerSesiones()) {
                                partidaSesion.adminCrearOvni(partes[1], partes[2]);
                            }
                        } else {
                            System.out.println("Uso: OVNI direccion puntos");
                        }
                        break;
                    case "velocidad":
                        if (partes.length >= 2) {
                            for (Partida partidaSesion : obtenerSesiones()) {
                                partidaSesion.adminVelocidad(partes[1]);
                            }
                        } else {
                            System.out.println("Uso: Velocidad valor");
                        }
                        break;
                    case "bunkers":
                        if (partes.length >= 2) {
                            for (Partida partidaSesion : obtenerSesiones()) {
                                partidaSesion.adminBunkers(partes[1]);
                            }
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
