#!/usr/bin/env python3
"""
Serveur HTTP d'exemple pour le USB Credential Provider
Retourne une liste de numéros de série USB autorisés (un par ligne)
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import json

# Liste des numéros de série USB autorisés
AUTHORIZED_SERIALS = [
    "AA1234567890",
    "BB0987654321",
    "CC1122334455",
]

class SerialHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        """Traite les requêtes GET et retourne la liste des serials"""

        # Vérifier le chemin
        if self.path == '/serials':
            # Retourner les serials (un par ligne)
            response = '\n'.join(AUTHORIZED_SERIALS)

            self.send_response(200)
            self.send_header('Content-Type', 'text/plain; charset=utf-8')
            self.send_header('Content-Length', len(response.encode('utf-8')))
            self.end_headers()

            self.wfile.write(response.encode('utf-8'))

            print(f"[INFO] Serials envoyés: {len(AUTHORIZED_SERIALS)} entrées")

        elif self.path == '/status':
            # Endpoint de status pour vérifier que le serveur fonctionne
            response = json.dumps({
                'status': 'ok',
                'authorized_count': len(AUTHORIZED_SERIALS)
            })

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', len(response))
            self.end_headers()

            self.wfile.write(response.encode('utf-8'))

        else:
            # 404 pour les autres chemins
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b'Not Found')

    def log_message(self, format, *args):
        """Log personnalisé"""
        print(f"[{self.log_date_time_string()}] {format % args}")


def run_server(host='0.0.0.0', port=8080):
    """Lance le serveur HTTP"""
    server_address = (host, port)
    httpd = HTTPServer(server_address, SerialHandler)

    print(f"""
╔════════════════════════════════════════════════════════╗
║   Serveur USB Credential Provider                     ║
╠════════════════════════════════════════════════════════╣
║   Adresse: http://{host}:{port}/serials            ║
║   Status:  http://{host}:{port}/status             ║
║                                                        ║
║   Serials autorisés: {len(AUTHORIZED_SERIALS)}                             ║
╚════════════════════════════════════════════════════════╝

[INFO] Serveur démarré. Appuyez sur Ctrl+C pour arrêter.
""")

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n[INFO] Arrêt du serveur...")
        httpd.shutdown()


if __name__ == '__main__':
    import sys

    # Permettre de spécifier le port en argument
    port = 8080
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            print(f"Port invalide: {sys.argv[1]}")
            sys.exit(1)

    run_server(port=port)
