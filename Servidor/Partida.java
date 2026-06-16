package Servidor;
import java.util.ArrayList;

public class Partida {
    
    private ArrayList<jugador> listaJugadores; 
    private ArrayList<Observadorsito> listaEspectadores; 
    private double velocidadAliens; 

    public Partida() {
        this.listaJugadores = new ArrayList<>();
        this.listaEspectadores = new ArrayList<>();
        this.velocidadAliens = 1.0; // velocidad base relajada
    }

    public void agregarJugador(jugador nuevoJugador) {
        listaJugadores.add(nuevoJugador);
    }

    //Observadorsito
    public void agregarEspectador(Observadorsito espectador) {
        listaEspectadores.add(espectador);
        espectador.recibirActualizacion("INFO,Bienvenido sapazo, acomodese a ver");
    }

    public void quitarEspectador(Observadorsito espectador) {
        listaEspectadores.remove(espectador);
    }

    // esto le escupe los datos a todos los clientes que tengan los observadorsito
    public void notificarTodos(String mensaje) {
        for (Observadorsito espectador : listaEspectadores) {
            espectador.recibirActualizacion(mensaje);
        }
    }

    //Flujo

    // Cuando un jugador pega un tiro y le da
    public void alienEliminado(jugador jugadorActual, String tipoAlien) {
        enemigo alienTemporal = enemigoFactory.nuevoEnemigo(tipoAlien, 0, 0);
        
        if (alienTemporal != null && jugadorActual != null) {
            jugadorActual.sumarPuntos(alienTemporal.puntos);
            notificarTodos("PUNTOS_J" + jugadorActual.idJugador + "," + jugadorActual.getPuntos());
        }
    }

    // Cuando el alien le da al jugador
    public void jugadorPierdeVida(jugador jugadorActual) {
        if (jugadorActual == null) return;
        
        jugadorActual.perderVida();
        notificarTodos("VIDAS_J" + jugadorActual.idJugador + "," + jugadorActual.cantidadVidas);
        
        if (jugadorActual.cantidadVidas <= 0) {
            notificarTodos("GAME_OVER_J" + jugadorActual.idJugador + ",Perdiste mae");
        }
    }

    // Cuando matan a todos los aliendos
    public void oleadaLimpiada(jugador jugadorActual) {
        if (jugadorActual != null) {
            jugadorActual.cantidadVidas += 1;
            notificarTodos("VIDAS_J" + jugadorActual.idJugador + "," + jugadorActual.cantidadVidas);
        }
        this.velocidadAliens += 0.5;
        notificarTodos("VELOCIDAD," + this.velocidadAliens);
    }

    //Comandos para probar

    public void adminCrearAlien(String x, String y, String pts) {
        notificarTodos("NUEVO_ALIEN," + x + "," + y + "," + pts);
    }

    public void adminCrearOvni(String direccion, String pts) {
        notificarTodos("NUEVO_OVNI," + direccion + "," + pts);
    }

    public void adminVelocidad(String velocidad) {
        notificarTodos("VELOCIDAD," + velocidad);
    }

    public void adminBunkers(String porcentaje) {
        // manda el string tipo "70%" a los clientes
        notificarTodos("BUNKERS," + porcentaje);
    }
}