package Servidor;

public class enemigo extends individuo{
    protected String ID;
    protected int puntos;
    
    public enemigo(int X, int Y, String ID, int puntos){
        super(X, Y);
        this.ID = ID;
        this.puntos = puntos;
    }

    @Override
    public void disparo(){}

    public String getID(){
        return this.ID;
    }

}