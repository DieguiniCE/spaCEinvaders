package Servidor;

public class enemigo extends individuo implements ataque{
    private String ID;
    protected int puntos;
    
    public enemigo(int X, int Y, String ID, int puntos){
        super(X, Y);
        this.ID = ID;
        this.puntos = puntos;
    }

    
    public void disparo(){}

    public String getID(){
        return this.ID;
    }

    public void setPuntos(int puntos){
        this.puntos = puntos;
    }

}