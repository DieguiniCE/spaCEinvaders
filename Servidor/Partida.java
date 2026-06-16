package Servidor;

import java.util.ArrayList;

public class Partida {

    private static final int ANCHO_PANTALLA = 800;
    private static final int ALTO_PANTALLA = 600;
    private static final int ANCHO_CANON = 40;
    private static final int VELOCIDAD_CANON = 5;

    private static final int ALIEN_ANCHO = 30;
    private static final int ALIEN_ALTO = 20;

    private static final class EstadoAlien {
        final int id;
        final int x;
        final int y;
        final int puntos;
        final int tipo;

        EstadoAlien(int id, int x, int y, int tipo, int puntos) {
            this.id = id;
            this.x = x;
            this.y = y;
            this.tipo = tipo;
            this.puntos = puntos;
        }
    }

    private final ArrayList<jugador> listaJugadores;
    private final ArrayList<Observadorsito> listaObservadores;
    private final ArrayList<EstadoAlien> listaAliens;
    private double velocidadAliens;
    private String estadoBunkers;
    private int contadorAliens;

    public Partida() {
        this.listaJugadores = new ArrayList<>();
        this.listaObservadores = new ArrayList<>();
        this.listaAliens = new ArrayList<>();
        this.velocidadAliens = 1.0;
        this.estadoBunkers = "100%";
        this.contadorAliens = 0;
    }

    public synchronized jugador crearJugador(int idJugador) {
        return new jugador(ANCHO_PANTALLA / 2, ALTO_PANTALLA - 50, 3, idJugador);
    }

    public synchronized void agregarJugador(jugador nuevoJugador) {
        if (nuevoJugador != null && !listaJugadores.contains(nuevoJugador)) {
            listaJugadores.add(nuevoJugador);
        }
    }

    public synchronized void registrarCliente(Observadorsito cliente, boolean esJugador, jugador jugadorAsignado) {
        listaObservadores.add(cliente);

        if (esJugador && jugadorAsignado != null) {
            cliente.recibirActualizacion("BIENVENIDA,J" + jugadorAsignado.idJugador);
        } else {
            cliente.recibirActualizacion("BIENVENIDA,SAPO");
        }

        for (jugador jugadorExistente : listaJugadores) {
            cliente.recibirActualizacion(
                "STATE_J" + jugadorExistente.idJugador + ","
                    + jugadorExistente.getposX() + ","
                    + jugadorExistente.getposY() + ","
                    + jugadorExistente.getVidas() + ","
                    + jugadorExistente.getPuntos()
            );
        }

        for (EstadoAlien alien : listaAliens) {
            cliente.recibirActualizacion(
                "NUEVO_ALIEN," + alien.id + "," + alien.x + "," + alien.y + "," + alien.puntos
            );
        }

        cliente.recibirActualizacion("VELOCIDAD," + this.velocidadAliens);
        cliente.recibirActualizacion("BUNKERS," + this.estadoBunkers);
    }

    public synchronized void quitarObservador(Observadorsito observador) {
        listaObservadores.remove(observador);
    }

    public synchronized void notificarTodos(String mensaje) {
        ArrayList<Observadorsito> copia = new ArrayList<>(listaObservadores);
        for (Observadorsito observador : copia) {
            observador.recibirActualizacion(mensaje);
        }
    }

    private void limitarPosicion(jugador jugadorActual) {
        int x = jugadorActual.getposX();
        int y = jugadorActual.getposY();

        if (x < 0) x = 0;
        if (x > ANCHO_PANTALLA - ANCHO_CANON) x = ANCHO_PANTALLA - ANCHO_CANON;
        if (y < 0) y = 0;
        if (y > ALTO_PANTALLA - 20) y = ALTO_PANTALLA - 20;

        jugadorActual.establecerPosicion(x, y);
    }

    public synchronized void moverJugador(jugador jugadorActual, String direccion) {
        if (jugadorActual == null) return;

        if (direccion.equals("L")) {
            jugadorActual.desplazamiento(-VELOCIDAD_CANON, 0);
        } else if (direccion.equals("R")) {
            jugadorActual.desplazamiento(VELOCIDAD_CANON, 0);
        }

        limitarPosicion(jugadorActual);
        enviarEstadoJugador(jugadorActual);
    }

    public synchronized void jugadorDispara(jugador jugadorActual) {
        if (jugadorActual == null) return;
        notificarTodos("DISPARO_J" + jugadorActual.idJugador + "," + jugadorActual.getposX());
    }

    public synchronized void enviarEstadoJugador(jugador jugadorActual) {
        if (jugadorActual == null) return;
        notificarTodos("STATE_J" + jugadorActual.idJugador + ","
            + jugadorActual.getposX() + ","
            + jugadorActual.getposY() + ","
            + jugadorActual.getVidas() + ","
            + jugadorActual.getPuntos());
    }

    private EstadoAlien buscarAlien(int idAlien) {
        for (EstadoAlien alien : listaAliens) {
            if (alien.id == idAlien) {
                return alien;
            }
        }
        return null;
    }

    private int tipoDesdePuntos(int puntos) {
        if (puntos >= 40) return 3;
        if (puntos >= 20) return 2;
        return 1;
    }

    private void agregarAlienInterno(int x, int y, int puntos) {
        int tipo = tipoDesdePuntos(puntos);
        EstadoAlien alien = new EstadoAlien(++contadorAliens, x, y, tipo, puntos);
        listaAliens.add(alien);
    }

    public synchronized void inicializarOleadaDefecto() {
        if (!listaAliens.isEmpty()) {
            return;
        }

        for (int fila = 0; fila < 4; fila++) {
            int puntos = 10;
            if (fila == 0) {
                puntos = 40;
            } else if (fila < 3) {
                puntos = 20;
            }

            for (int columna = 0; columna < 10; columna++) {
                int x = 100 + (columna * 48);
                int y = 60 + (fila * 34);
                agregarAlienInterno(x, y, puntos);
            }
        }

        for (EstadoAlien alien : listaAliens) {
            notificarTodos("NUEVO_ALIEN," + alien.id + "," + alien.x + "," + alien.y + "," + alien.puntos);
        }
        notificarTodos("VELOCIDAD," + velocidadAliens);
        notificarTodos("BUNKERS," + estadoBunkers);
    }

    public synchronized void alienEliminado(jugador jugadorActual, int idAlien, String tipoAlien) {
        EstadoAlien alien = buscarAlien(idAlien);
        if (alien == null && tipoAlien != null) {
            for (EstadoAlien candidato : listaAliens) {
                if ((tipoAlien.equalsIgnoreCase("calamar") && candidato.tipo == 1)
                    || (tipoAlien.equalsIgnoreCase("cangrejo") && candidato.tipo == 2)
                    || (tipoAlien.equalsIgnoreCase("pulpo") && candidato.tipo == 3)
                    || (tipoAlien.equalsIgnoreCase("ovni"))) {
                    alien = candidato;
                    break;
                }
            }
        }

        if (alien != null) {
            listaAliens.remove(alien);
            notificarTodos("BORRAR_ALIEN," + alien.id);

            if (jugadorActual != null) {
                jugadorActual.sumarPuntos(alien.puntos);
                notificarTodos("PUNTOS_J" + jugadorActual.idJugador + "," + jugadorActual.getPuntos());
                enviarEstadoJugador(jugadorActual);
            }

            if (listaAliens.isEmpty()) {
                oleadaLimpiada(jugadorActual);
            }
        }
    }

    public synchronized void jugadorPierdeVida(jugador jugadorActual) {
        if (jugadorActual == null) return;

        jugadorActual.perderVida();
        notificarTodos("VIDAS_J" + jugadorActual.idJugador + "," + jugadorActual.getVidas());

        if (jugadorActual.getVidas() <= 0) {
            notificarTodos("GAME_OVER_J" + jugadorActual.idJugador + ",Perdiste mae");
        } else {
            enviarEstadoJugador(jugadorActual);
        }
    }

    public synchronized void oleadaLimpiada(jugador jugadorActual) {
        if (jugadorActual != null) {
            jugadorActual.cantidadVidas += 1;
            notificarTodos("VIDAS_J" + jugadorActual.idJugador + "," + jugadorActual.getVidas());
        }

        this.velocidadAliens += 0.5;
        notificarTodos("VELOCIDAD," + this.velocidadAliens);

        if (jugadorActual != null) {
            enviarEstadoJugador(jugadorActual);
        }

        inicializarOleadaDefecto();
    }

    public synchronized void adminCrearAlien(String x, String y, String pts) {
        try {
            int posX = Integer.parseInt(x);
            int posY = Integer.parseInt(y);
            int puntos = Integer.parseInt(pts);
            agregarAlienInterno(posX, posY, puntos);
            notificarTodos("NUEVO_ALIEN," + contadorAliens + "," + posX + "," + posY + "," + puntos);
        } catch (NumberFormatException e) {
            System.out.println("No se pudo crear alien: " + e.getMessage());
        }
    }

    public synchronized void adminCrearOvni(String direccion, String pts) {
        notificarTodos("NUEVO_OVNI," + direccion + "," + pts);
    }

    public synchronized void adminVelocidad(String velocidad) {
        try {
            this.velocidadAliens = Double.parseDouble(velocidad);
        } catch (NumberFormatException e) {
            this.velocidadAliens = 1.0;
        }
        notificarTodos("VELOCIDAD," + this.velocidadAliens);
    }

    public synchronized void adminBunkers(String porcentaje) {
        this.estadoBunkers = porcentaje;
        notificarTodos("BUNKERS," + porcentaje);
    }
}
