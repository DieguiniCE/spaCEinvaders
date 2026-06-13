package Servidor;

public class bunker extends individuo{
    protected int cantidadVidas;

    public bunker(int X, int Y, int vidas){    
        super(X, Y);
        this.cantidadVidas = vidas;
    }

    public void perderVida(){
        if (cantidadVidas >= 1){
            this.cantidadVidas -=1;
        }
        else {this.conVida = false;}
    }

    @Override
    public void disparo(){}
}
