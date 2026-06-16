package Servidor;
import java.util.ArrayList;

public class Partida {

    private static final int ANCHO_PANTALLA = 800;
    private static final int ALTO_PANTALLA = 600;
    private static final int ANCHO_CANON = 40;
    private static final int VELOCIDAD_CANON = 5;

    private ArrayList<jugador> listaJugadores;
    private ArrayList<Observadorsito> listaObservadores;
    private double velocidadAliens;
    private int contadorAliens;

    public Partida() {
        this.listaJugadores = new ArrayList<>();
        this.listaObservadores = new ArrayList<>();
        this.velocidadAliens = 1.0;
        this.contadorAliens = 0;
    }

    public void agregarJugador(jugador nuevoJugador) {
        listaJugadores.add(nuevoJugador);
    }

    public void registrarCliente(Observadorsito cliente, boolean esJugador, jugador jugadorAsignado) {
        listaObservadores.add(cliente);
        if (esJugador && jugadorAsignado != null) {
            cliente.recibirActualizacion("BIENVENIDA,J" + jugadorAsignado.idJugador);
            enviarEstadoJugador(jugadorAsignado);
        } else {
            cliente.recibirActualizacion("BIENVENIDA,SAPO");
        }
    }

    public void quitarObservador(Observadorsito observador) {
        listaObservadores.remove(observador);
    }

    public void notificarTodos(String mensaje) {
        for (Observadorsito observador : listaObservadores) {
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

    public void moverJugador(jugador jugadorActual, String direccion) {
        if (jugadorActual == null) return;

        if (direccion.equals("L")) {
            jugadorActual.desplazamiento(-VELOCIDAD_CANON, 0);
        } else if (direccion.equals("R")) {
            jugadorActual.desplazamiento(VELOCIDAD_CANON, 0);
        }

        limitarPosicion(jugadorActual);
        enviarEstadoJugador(jugadorActual);
    }

    public void jugadorDispara(jugador jugadorActual) {
        if (jugadorActual == null) return;
        notificarTodos("DISPARO_J" + jugadorActual.idJugador + "," + jugadorActual.getposX());
    }

    public void enviarEstadoJugador(jugador jugadorActual) {
        if (jugadorActual == null) return;
        notificarTodos("STATE_J" + jugadorActual.idJugador + ","
            + jugadorActual.getposX() + ","
            + jugadorActual.getposY() + ","
            + jugadorActual.getVidas() + ","
            + jugadorActual.getPuntos());
    }

    public void alienEliminado(jugador jugadorActual, String tipoAlien) {
        enemigo alienTemporal = enemigoFactory.nuevoEnemigo(tipoAlien, 0, 0);

        if (alienTemporal != null && jugadorActual != null) {
            jugadorActual.sumarPuntos(alienTemporal.puntos);
            notificarTodos("PUNTOS_J" + jugadorActual.idJugador + "," + jugadorActual.getPuntos());
            enviarEstadoJugador(jugadorActual);
        }
    }

    public void jugadorPierdeVida(jugador jugadorActual) {
        if (jugadorActual == null) return;

        jugadorActual.perderVida();
        notificarTodos("VIDAS_J" + jugadorActual.idJugador + "," + jugadorActual.getVidas());

        if (jugadorActual.getVidas() <= 0) {
            notificarTodos("GAME_OVER_J" + jugadorActual.idJugador + ",Perdiste mae");
        } else {
            enviarEstadoJugador(jugadorActual);
        }
    }

    public void oleadaLimpiada(jugador jugadorActual) {
        if (jugadorActual != null) {
            jugadorActual.cantidadVidas += 1;
            notificarTodos("VIDAS_J" + jugadorActual.idJugador + "," + jugadorActual.getVidas());
        }
        this.velocidadAliens += 0.5;
        notificarTodos("VELOCIDAD," + this.velocidadAliens);
        if (jugadorActual != null) {
            enviarEstadoJugador(jugadorActual);
        }
    }

    public void adminCrearAlien(String x, String y, String pts) {
        contadorAliens++;
        notificarTodos("NUEVO_ALIEN," + contadorAliens + "," + x + "," + y + "," + pts);
    }

    public void adminCrearOvni(String direccion, String pts) {
        notificarTodos("NUEVO_OVNI," + direccion + "," + pts);
    }

    public void adminVelocidad(String velocidad) {
        try {
            this.velocidadAliens = Double.parseDouble(velocidad);
        } catch (NumberFormatException e) {
            this.velocidadAliens = 1.0;
        }
        notificarTodos("VELOCIDAD," + this.velocidadAliens);
    }

    public void adminBunkers(String porcentaje) {
        notificarTodos("BUNKERS," + porcentaje);
    }
}
