package Servidor;

public abstract class individuo {
    protected boolean conVida;
    protected int posX;
    protected int posY;

    public individuo(int X, int Y){
        this.posX = X;
        this.posY = Y;
        this.conVida = true;
    }

    public void desplazamiento(int nuevaX, int nuevaY){
        if (conVida){
            this.posX += nuevaX;
            this.posY += nuevaY;
        }
    }

    abstract public void disparo();

    public int getposX(){
        return this.posX;
    }

    public int getposY(){
        return this.posY;
    }


}
