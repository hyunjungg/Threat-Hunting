# Set up for C2 Server

| OS     | Version |
| :----- | :------ |
| Ubuntu | 18.04   |

To set up the C2 server, follow the steps below

1. Clone the GitHub repository:

   ```bash
   git clone https://github.com/hyunjungg/Threat-Hunting.git
   ```

2. Change directory to the APT37 Resources folder:

   ```bash
   cd Threat-Hunting/APT37/Resources
   ```

3. Start the Docker container:

   ```bash
   docker run --name apt37_server -d -p 80:80 -v $(pwd)/Compromised_Server/:/var/www/html hyunjungg/apt37:1
   ```

If you want to view the Dockerfile used to build the images, please refer to this [link](https://github.com/hyunjungg/Threat-Hunting/blob/APT37/APT37/Setup/Dockerfile)!

# Set up for Victim

| OS      | Version |
| :------ | :------ |
| Windows | 1809    |

To set up the Victim System, check the configuration below

* Windows Defender OFF

* Add the following line to the file located at `c:\Windows\System32\drivers\etc\hosts`

  ```bash
  compromised.server [server_ip]
  ```

  

