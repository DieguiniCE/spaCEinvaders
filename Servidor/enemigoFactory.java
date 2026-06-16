package Servidor;
import java.util.Random;

public class enemigoFactory {

    // acá armamos los aliens dependiendo de lo que manden
    public static enemigo nuevoEnemigo(String tipo, int x, int y){
        int puntos = 0;

        switch (tipo.toLowerCase()) {
            case "calamar":
                puntos = 10;
                break;
            
            case "cangrejo":
                puntos = 20;
                break;

            case "pulpo":
                puntos = 40;
                break;

            case "ovni":
                Random randomPuntos = new Random();
                puntos = randomPuntos.nextInt(251) + 50; //entre 50 y 300
                break;
                
            default:
                System.out.println("Papi escoja bien ese alien, no existe");
                break;
        }

        return new enemigo(x, y, tipo, puntos);
    }
}