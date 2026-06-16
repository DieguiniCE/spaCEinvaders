package Servidor;

import java.util.ArrayList;
import java.util.Random;

public class Partida {

    private static final int ANCHO_PANTALLA = 800;
    private static final int ALTO_PANTALLA = 600;
    private static final int ANCHO_CANON = 40;
    private static final int VELOCIDAD_CANON = 5;

    private static final int ALIEN_ANCHO = 30;
    private static final int ALIEN_ALTO = 20;

    private static final class EstadoOvni {
        final int id;
        int x;
        int y;
        int direccion;
        int puntos;

        EstadoOvni(int id, int x, int y, int direccion, int puntos) {
            this.id = id;
            this.x = x;
            this.y = y;
            this.direccion = direccion;
            this.puntos = puntos;
        }
    }

    private static final class EstadoAlien {
        final int id;
        int x;
        int y;
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
    private final boolean[] jugadoresOcupados;
    private final int[] bunkerSalud;
    private final Random random;
    private final int idSesion;
    private double velocidadAliens;
    private String estadoBunkers;
    private int contadorAliens;
    private int contadorOvni;
    private EstadoOvni ovniActual;
    private int direccionAliens;

    public Partida() {
        this(0);
    }

    public Partida(int idSesion) {
        this.listaJugadores = new ArrayList<>();
        this.listaObservadores = new ArrayList<>();
        this.listaAliens = new ArrayList<>();
        this.jugadoresOcupados = new boolean[] {false, false, false};
        this.bunkerSalud = new int[] {100, 100, 100, 100};
        this.random = new Random();
        this.idSesion = idSesion;
        this.velocidadAliens = 1.0;
        this.estadoBunkers = "100,100,100,100";
        this.contadorAliens = 0;
        this.contadorOvni = 0;
        this.ovniActual = null;
        this.direccionAliens = 1;

        Thread hiloEventos = new Thread(this::bucleEventosAutomaticos, "PartidaEventos-" + idSesion);
        hiloEventos.setDaemon(true);
        hiloEventos.start();
    }

    public int getIdSesion() {
        return idSesion;
    }

    public synchronized jugador asignarJugador() {
        return asignarJugador(1, 1);
    }

    public synchronized jugador asignarJugador(int maxJugadores, int jugadorPreferido) {
        if (maxJugadores <= 1) {
            int idJugador = (jugadorPreferido == 2) ? 2 : 1;
            if (!jugadoresOcupados[idJugador]) {
                return ocuparJugador(idJugador);
            }
            return null;
        }

        for (int idJugador = 1; idJugador <= 2 && idJugador < jugadoresOcupados.length; idJugador++) {
            if (!jugadoresOcupados[idJugador]) {
                return ocuparJugador(idJugador);
            }
        }

        return null;
    }

    private jugador ocuparJugador(int idJugador) {
        jugadoresOcupados[idJugador] = true;
        jugador nuevoJugador = new jugador(ANCHO_PANTALLA / 2, ALTO_PANTALLA - 50, 3, idJugador);
        listaJugadores.add(nuevoJugador);
        return nuevoJugador;
    }

    public synchronized void liberarJugador(jugador jugadorAbandonado) {
        if (jugadorAbandonado == null) {
            return;
        }

        int idJugador = jugadorAbandonado.idJugador;
        if (idJugador > 0 && idJugador < jugadoresOcupados.length) {
            jugadoresOcupados[idJugador] = false;
        }

        listaJugadores.remove(jugadorAbandonado);
        notificarTodos("BORRAR_JUGADOR," + idJugador);
    }

    public synchronized void registrarCliente(Observadorsito cliente, boolean esJugador, jugador jugadorAsignado) {
        listaObservadores.add(cliente);
        cliente.recibirActualizacion("SESION," + this.idSesion);

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
        if (ovniActual != null) {
            cliente.recibirActualizacion(
                "NUEVO_OVNI," + ovniActual.id + "," + ovniActual.x + "," + ovniActual.y + "," + ovniActual.direccion + "," + ovniActual.puntos
            );
        }
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

    private void bucleEventosAutomaticos() {
        while (true) {
            try {
                Thread.sleep(150);
            } catch (InterruptedException error) {
                Thread.currentThread().interrupt();
                return;
            }

            synchronized (this) {
                actualizarAliens();
                actualizarOvni();
            }
        }
    }

    private void actualizarAliens() {
        if (listaAliens.isEmpty()) {
            return;
        }

        int paso = (int) Math.max(1, Math.round(velocidadAliens));
        int minX = ANCHO_PANTALLA;
        int maxX = 0;

        for (EstadoAlien alien : listaAliens) {
            if (alien.x < minX) {
                minX = alien.x;
            }
            if (alien.x > maxX) {
                maxX = alien.x;
            }
        }

        boolean golpeaBorde = (direccionAliens > 0 && maxX + ALIEN_ANCHO + paso >= ANCHO_PANTALLA - 20)
            || (direccionAliens < 0 && minX - paso <= 20);

        if (golpeaBorde) {
            direccionAliens *= -1;
            for (EstadoAlien alien : listaAliens) {
                alien.y += 16;
            }
        } else {
            for (EstadoAlien alien : listaAliens) {
                alien.x += (paso * direccionAliens);
            }
        }

        ArrayList<EstadoAlien> eliminados = new ArrayList<>();

        for (EstadoAlien alien : listaAliens) {
            if (alien.y + ALIEN_ALTO >= ALTO_PANTALLA - 55) {
                jugador jugadorObjetivo = jugadorMasCercano(alien.x, alien.y);
                if (jugadorObjetivo != null) {
                    jugadorPierdeVida(jugadorObjetivo);
                }
                eliminados.add(alien);
                continue;
            }

            for (int i = 0; i < bunkerSalud.length; i++) {
                if (!bunkerActivo(i)) {
                    continue;
                }

                int bunkerX = bunkerX(i);
                int bunkerY = ALTO_PANTALLA - 150;
                boolean chocaBunker = alien.x < bunkerX + 60 && alien.x + ALIEN_ANCHO > bunkerX
                    && alien.y < bunkerY + 35 && alien.y + ALIEN_ALTO > bunkerY;
                if (chocaBunker) {
                    bunkerGolpeado(i, 15);
                    eliminados.add(alien);
                    break;
                }
            }
        }

        if (!eliminados.isEmpty()) {
            listaAliens.removeAll(eliminados);
            for (EstadoAlien alien : eliminados) {
                notificarTodos("BORRAR_ALIEN," + alien.id);
            }

            if (listaAliens.isEmpty()) {
                oleadaLimpiada(null);
            }
        }

        for (EstadoAlien alien : listaAliens) {
            notificarTodos("MOVER_ALIEN," + alien.id + "," + alien.x + "," + alien.y);
        }
    }

    private jugador jugadorMasCercano(int x, int y) {
        jugador mejor = null;
        int mejorDistancia = Integer.MAX_VALUE;

        for (jugador candidato : listaJugadores) {
            int distancia = Math.abs(candidato.getposX() - x) + Math.abs(candidato.getposY() - y);
            if (distancia < mejorDistancia) {
                mejorDistancia = distancia;
                mejor = candidato;
            }
        }

        return mejor;
    }

    private int bunkerX(int indice) {
        switch (indice) {
            case 0: return 120;
            case 1: return 275;
            case 2: return 430;
            case 3: return 585;
            default: return 0;
        }
    }

    private void actualizarOvni() {
        if (ovniActual == null) {
            if (random.nextInt(70) == 0) {
                crearOvniAleatorio();
            }
            return;
        }

        ovniActual.x += ovniActual.direccion * 8;
        if (ovniActual.x < -40 || ovniActual.x > ANCHO_PANTALLA + 40) {
            notificarTodos("BORRAR_OVNI," + ovniActual.id);
            ovniActual = null;
            return;
        }

        notificarTodos("MOVER_OVNI," + ovniActual.id + "," + ovniActual.x + "," + ovniActual.y);
    }

    private void crearOvniAleatorio() {
        int direccion = random.nextBoolean() ? 1 : -1;
        int x = (direccion > 0) ? -40 : (ANCHO_PANTALLA + 10);
        int puntos = 50 + random.nextInt(251);
        ovniActual = new EstadoOvni(++contadorOvni, x, 25, direccion, puntos);

        notificarTodos(
            "NUEVO_OVNI," + ovniActual.id + "," + ovniActual.x + "," + ovniActual.y + "," + ovniActual.direccion + "," + ovniActual.puntos
        );
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
        String nombreTipo = tipo == 3 ? "pulpo" : (tipo == 2 ? "cangrejo" : "calamar");
        enemigo alienBase = enemigoFactory.nuevoEnemigo(nombreTipo, x, y);
        EstadoAlien alien = new EstadoAlien(++contadorAliens, alienBase.getposX(), alienBase.getposY(), tipo, alienBase.getPuntos());
        listaAliens.add(alien);
    }

    public synchronized void inicializarOleadaDefecto() {
        if (!listaAliens.isEmpty()) {
            return;
        }

        this.direccionAliens = 1;

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

    public synchronized void ovniEliminado(jugador jugadorActual, int idOvni) {
        if (ovniActual == null || ovniActual.id != idOvni) {
            return;
        }

        int puntosExtras = ovniActual.puntos;
        ovniActual = null;
        notificarTodos("BORRAR_OVNI," + idOvni);

        if (jugadorActual != null) {
            jugadorActual.sumarPuntos(puntosExtras);
            notificarTodos("PUNTOS_J" + jugadorActual.idJugador + "," + jugadorActual.getPuntos());
            enviarEstadoJugador(jugadorActual);
        }
    }

    public synchronized void bunkerGolpeado(int indice, int dano) {
        if (indice < 0 || indice >= bunkerSalud.length) {
            return;
        }

        bunkerSalud[indice] -= dano;
        if (bunkerSalud[indice] < 0) {
            bunkerSalud[indice] = 0;
        }

        actualizarEstadoBunkers();
    }

    public synchronized boolean bunkerActivo(int indice) {
        return indice >= 0 && indice < bunkerSalud.length && bunkerSalud[indice] > 0;
    }

    public synchronized int estadoBunker(int indice) {
        if (indice < 0 || indice >= bunkerSalud.length) {
            return 0;
        }
        return bunkerSalud[indice];
    }

    private void actualizarEstadoBunkers() {
        this.estadoBunkers = bunkerSalud[0] + "," + bunkerSalud[1] + "," + bunkerSalud[2] + "," + bunkerSalud[3];
        notificarTodos("BUNKERS," + this.estadoBunkers);
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
        try {
            int dir = direccion.equalsIgnoreCase("R") || direccion.equals("1") ? 1 : -1;
            int puntos = Integer.parseInt(pts);
            ovniActual = new EstadoOvni(++contadorOvni, dir > 0 ? -40 : (ANCHO_PANTALLA + 10), 25, dir, puntos);
            notificarTodos(
                "NUEVO_OVNI," + ovniActual.id + "," + ovniActual.x + "," + ovniActual.y + "," + ovniActual.direccion + "," + ovniActual.puntos
            );
        } catch (NumberFormatException e) {
            System.out.println("No se pudo crear OVNI: " + e.getMessage());
        }
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
        try {
            String limpio = porcentaje.replace("%", "");
            int valor = Integer.parseInt(limpio);
            if (valor < 0) {
                valor = 0;
            }
            if (valor > 100) {
                valor = 100;
            }

            for (int i = 0; i < bunkerSalud.length; i++) {
                bunkerSalud[i] = valor;
            }

            actualizarEstadoBunkers();
        } catch (NumberFormatException e) {
            System.out.println("No se pudo actualizar bunkers: " + e.getMessage());
        }
    }
}
