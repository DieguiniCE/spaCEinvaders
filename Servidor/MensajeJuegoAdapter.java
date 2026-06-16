package Servidor;

public class MensajeJuegoAdapter {

    private final String mensajeBruto;
    private final String[] partes;

    public MensajeJuegoAdapter(String mensajeBruto) {
        this.mensajeBruto = mensajeBruto == null ? "" : mensajeBruto.trim();
        this.partes = this.mensajeBruto.isEmpty() ? new String[0] : this.mensajeBruto.split(",");
    }

    public boolean esRolEspectador() {
        return "ROL,ESPECTADOR".equalsIgnoreCase(mensajeBruto);
    }

    public boolean esRolJugador() {
        return "ROL,JUGADOR".equalsIgnoreCase(mensajeBruto);
    }

    public String comando() {
        return partes.length > 0 ? partes[0] : "";
    }

    public boolean esComando(String nombre) {
        return comando().equalsIgnoreCase(nombre);
    }

    public boolean tienePartes(int cantidadMinima) {
        return partes.length >= cantidadMinima;
    }

    public String parte(int indice) {
        if (indice < 0 || indice >= partes.length) {
            return "";
        }

        return partes[indice];
    }
}