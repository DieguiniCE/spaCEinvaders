package Servidor;

public class jugador extends individuo implements ataque {
    protected int cantidadPuntos;
    protected int cantidadVidas;

    public jugador(int X, int Y, int vidas){
        super(X, Y);
        this.cantidadPuntos = 0;
        this.cantidadVidas = vidas;
    }

    public void sumarPuntos(int puntos){
        this.cantidadPuntos += puntos;
    }
    
    public int getPuntos(){
        return this.cantidadPuntos;
    }

    public void perderVida(){
        if (cantidadVidas >= 1){
            this.cantidadVidas -=1;
        }
        else {this.conVida = false;}
    }

    public void disparo(){}
}
