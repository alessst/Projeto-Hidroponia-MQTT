import csv
import paho.mqtt.client as mqtt

# Configurações do servidor MQTT
mqtt_server = "your_mqtt_server"
mqtt_port = 1883
mqtt_topic_temperature = "sensor/temperature"
mqtt_topic_moisture = "sensor/moisture"

# Nome do arquivo CSV
csv_filename = "sensor_data.csv"

# Função de callback para quando a conexão MQTT for estabelecida
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe(mqtt_topic_temperature)
    client.subscribe(mqtt_topic_moisture)

# Função de callback para quando uma mensagem MQTT for recebida
def on_message(client, userdata, msg):
    with open(csv_filename, mode='a', newline='') as file:
        writer = csv.writer(file)
        if msg.topic == mqtt_topic_temperature:
            temperature = msg.payload.decode('utf-8')
            writer.writerow(["Temperature", temperature])
            print(f"Temperature: {temperature} C")
        elif msg.topic == mqtt_topic_moisture:
            moisture = msg.payload.decode('utf-8')
            writer.writerow(["Moisture", moisture])
            print(f"Soil Moisture: {moisture}")

# Configura o cliente MQTT
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Conecta ao servidor MQTT
client.connect(mqtt_server, mqtt_port, 60)

# Loop para processar mensagens e manter a conexão
client.loop_forever()
