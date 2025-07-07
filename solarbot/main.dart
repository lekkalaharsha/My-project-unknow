import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp();
  runApp(SmartCarApp());
}

class SmartCarApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Smart Car Controller',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(primarySwatch: Colors.blue),
      home: CarControlPage(),
    );
  }
}

class CarControlPage extends StatefulWidget {
  @override
  State<CarControlPage> createState() => _CarControlPageState();
}

class _CarControlPageState extends State<CarControlPage> {
  final dbRef = FirebaseDatabase.instance.reference().child('FirebaseIOT');
  String carStatus = "Unknown";
  double frontDist = 0;
  double backDist = 0;
  double speed = 128;

  @override
  void initState() {
    super.initState();
    dbRef.child('status').onValue.listen((event) {
      setState(() {
        carStatus = event.snapshot.value?.toString() ?? "Unknown";
      });
    });
    dbRef.child('front_distance').onValue.listen((event) {
      setState(() {
        frontDist = double.tryParse(event.snapshot.value?.toString() ?? "0") ?? 0;
      });
    });
    dbRef.child('back_distance').onValue.listen((event) {
      setState(() {
        backDist = double.tryParse(event.snapshot.value?.toString() ?? "0") ?? 0;
      });
    });
    dbRef.child('speed').onValue.listen((event) {
      setState(() {
        speed = double.tryParse(event.snapshot.value?.toString() ?? "128") ?? 128;
      });
    });
  }

  void sendCommand(String command) {
    dbRef.child('man').set(command);
    dbRef.child('speed').set(speed.toInt());
  }

  Widget _buildStatusCard() {
    return Card(
      elevation: 3,
      child: Padding(
        padding: EdgeInsets.all(16),
        child: Column(
          children: [
            Text('Status: $carStatus', style: TextStyle(fontSize: 22, fontWeight: FontWeight.bold)),
            SizedBox(height: 12),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                Text('Front: ${frontDist.toStringAsFixed(1)} cm', style: TextStyle(fontSize: 16)),
                Text('Back: ${backDist.toStringAsFixed(1)} cm', style: TextStyle(fontSize: 16)),
              ],
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildControls() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      children: [
        ElevatedButton.icon(
          icon: Icon(Icons.arrow_upward),
          label: Text('Forward'),
          onPressed: () => sendCommand("1"),
        ),
        ElevatedButton.icon(
          icon: Icon(Icons.stop),
          label: Text('Stop'),
          style: ElevatedButton.styleFrom(primary: Colors.red),
          onPressed: () => sendCommand("0"),
        ),
        ElevatedButton.icon(
          icon: Icon(Icons.arrow_downward),
          label: Text('Backward'),
          onPressed: () => sendCommand("2"),
        ),
      ],
    );
  }

  Widget _buildSpeedSlider() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text('Speed: ${speed.toInt()}'),
        Slider(
          min: 0, max: 255,
          value: speed,
          onChanged: (val) {
            setState(() => speed = val);
            dbRef.child('speed').set(speed.toInt());
          },
        ),
      ],
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('Smart Car Controller')),
      body: Padding(
        padding: EdgeInsets.all(16),
        child: Column(
          children: [
            _buildStatusCard(),
            SizedBox(height: 28),
            _buildControls(),
            SizedBox(height: 24),
            _buildSpeedSlider(),
          ],
        ),
      ),
    );
  }
}
