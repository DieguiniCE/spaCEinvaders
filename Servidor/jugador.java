package Servidor;

public class jugador extends individuo implements ataque {
    protected int cantidadPuntos;
    protected int cantidadVidas;
    protected int idJugador; 

    public jugador(int X, int Y, int vidas, int idJugador){
        super(X, Y);
        this.cantidadPuntos = 0; 
        this.cantidadVidas = vidas;
        this.idJugador = idJugador;
    }

    public void sumarPuntos(int puntos){
        this.cantidadPuntos += puntos; 
    }
    
    public int getPuntos(){
        return this.cantidadPuntos;
    }

    public void perderVida(){
        if (cantidadVidas >= 1){
            this.cantidadVidas -= 1; 
        }
        else {
            this.conVida = false; // ya mamó el compa
        }
    }

    public void disparo(){} 
}