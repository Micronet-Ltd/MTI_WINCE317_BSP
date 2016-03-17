using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Threading;
using System.Net;
using System.IO;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace TCPIPServer
{
    public partial class MainForm : Form
    {
        #region Internal definitions
        public enum MessageType
        {
            None,
            Info,
            Communication,
            Written,
            Completed,
            Error,
            Replace
        }

        public struct ThreadInfo
        {
            [DefaultValue(null)]
            public TcpClient Client { get; set; }
            [DefaultValue(null)]
            public ManualResetEvent WaitEvent { get; set; }
            [DefaultValue("")]
            public string Address { get; set; }
            [DefaultValue(null)]
            public Thread ClientThread { get; set; }
        }

        public enum PacketHeaderType
        {
            InitializationHeader = 0,
            InitializationData = 1,
            StopIterations = 2,
            EndOfTest = 3,
            NextIteration = 4
        }

        public struct PacketHeader
        {
            public PacketHeaderType type;
            public int size;
        }

        public struct PacketHeaderData
        {
            public int bufferSize;
            public int streamSize;
            public int irerations;
            public int direction;
        }

        public struct PacketAcnowlege
        {
            public int ack;
            public int received;
        }

        public enum TransferDirection
        {
            Upload,
            Download,
            Both,
            Unknown
        }

        public struct PacketBeginIterationHeader
        {
            public int iterationNumber;
            public TransferDirection direction;
        }

        public enum SocketStates
        {
            PendingHeader,
            PendingHeaderData,
            PendingIterationBegin,
            ReceiveData,
            SendAck,
            SendData,
            CloseConnection
        }

        #endregion

        #region Properties
        private TcpListener server = null;
        private Thread listenThread = null;
        private Dictionary<string, ThreadInfo> threadHold = new Dictionary<string, ThreadInfo>();
        private Mutex serverMutex = null;
        private string logSource = "TCP/IP QAA Server";
        #endregion

        #region Constants
        readonly string fileToSendFullName = @"C:\Develop\Test.pdf";
        #endregion

        public MainForm()
        {
            InitializeComponent();
        }

        private void buttonTurnOn_Click(object sender, EventArgs e)
        {
            try
            {
                if (server == null)
                {
                    StartServer();
                }
                else
                {
                    StopServer();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        public void ReplaceMessage(object info, string text, MessageType type)
        {
            if ((listMessages.IsDisposed) || !(info is ThreadInfo))
                return;

            ThreadInfo threadInfo = (ThreadInfo)info;
            foreach (ListViewItem item in listMessages.Items)
            {
                if ((item.SubItems.Count > 2) && (item.SubItems[2].Text == threadInfo.Address))
                {
                    SetMessageLine(text, item);

                    return;
                }
            }

            // Line not found
            AddMessage(info, text, type);
        }

        private void SetMessageLine(string text, ListViewItem item)
        {
            item.Text = DateTime.Now.ToLongTimeString();
            item.SubItems[1].Text = text;

            listMessages.Columns[0].Width = -1;
            listMessages.Columns[1].Width = -1;
        }

        public int AddMessage(object info, string text, MessageType type)
        {
            if (listMessages.IsDisposed)
                return -1;

            ListViewItem lvi = null;

            if (type == MessageType.Replace)
            {
                lvi = listMessages.Items[0];
                lvi.Text = DateTime.Now.ToLongTimeString();
            }
            else
            {
                lvi = listMessages.Items.Insert(0, DateTime.Now.ToLongTimeString());
            }

            switch (type)
            {
                case MessageType.Info:
                    lvi.ImageIndex = 2;
                    lvi.ForeColor = Color.DarkBlue;
                    notifyTrayIcon.ShowBalloonTip(1000, type.ToString(), text, ToolTipIcon.Info);

                    EventLog.WriteEntry(logSource, text, EventLogEntryType.Information);
                    break;
                case MessageType.Communication:
                    lvi.ImageIndex = 0;
                    break;
                case MessageType.Written:
                    lvi.ImageIndex = 1;
                    break;
                case MessageType.Completed:
                    lvi.ImageIndex = 3;
                    lvi.ForeColor = Color.DarkGreen;
                    notifyTrayIcon.ShowBalloonTip(1000, type.ToString(), text, ToolTipIcon.Info);

                    EventLog.WriteEntry(logSource, text, EventLogEntryType.Information);
                    break;
                case MessageType.Error:
                    lvi.ImageIndex = 4;
                    lvi.ForeColor = Color.Red;
                    lvi.Font = new Font(lvi.Font, FontStyle.Bold);
                    notifyTrayIcon.ShowBalloonTip(1000, type.ToString(), text, ToolTipIcon.Error);

                    EventLog.WriteEntry(logSource, text, EventLogEntryType.Error);
                    break;
                case MessageType.Replace:
                    break;
                default:
                    lvi.ImageIndex = -1;
                    break;
            }

            if (type == MessageType.Replace)
                lvi.SubItems[1].Text = text;
            else
                lvi.SubItems.Add(text);

            listMessages.Columns[0].Width = -1;
            listMessages.Columns[1].Width = -1;

            if (info is ThreadInfo)
            {
                lvi.SubItems.Add(((ThreadInfo)info).Address);
            };

            return lvi.Index;
        }

        private void StopServer()
        {
            if (server != null)
            {
                buttonTurnOn.Text = "Start";

                foreach (var info in threadHold)
                {
                    info.Value.WaitEvent.Set();
                    info.Value.ClientThread.Abort();
                }
                threadHold.Clear();

                server.Stop();
                server = null;

                AddMessage(this, "Server stopped", MessageType.Info);
                textPort.Enabled = true;
                labelPort.Enabled = true;
            }
        }

        private void StartServer()
        {
            int port;
            if (!int.TryParse(textPort.Text, out port))
            {
                port = 5000;
                errorProvider.SetIconAlignment(labelPort, ErrorIconAlignment.MiddleLeft);
                errorProvider.SetError(labelPort, "Wrong port number, using default 5000");
            }

            server = new TcpListener(IPAddress.Any, port);

            listenThread = new Thread(new ThreadStart(ListenForClients));
            listenThread.Start();

            AddMessage(this, "Server started at port " + port.ToString() + ".", MessageType.Info);

            buttonTurnOn.Text = "Stop";
            textPort.Enabled = false;
            labelPort.Enabled = false;
        }

        private void ListenForClients()
        {
            server.Start();

            while (true)
            {
                try
                {
                    TcpClient tcpClient = server.AcceptTcpClient();
                    tcpClient.ReceiveTimeout = 30000;
                    tcpClient.ReceiveBufferSize = 32768;

                    string ip = ((IPEndPoint)tcpClient.Client.RemoteEndPoint).Address.ToString();
                    string port = ((IPEndPoint)tcpClient.Client.RemoteEndPoint).Port.ToString();

                    this.Invoke((Action)delegate() { AddMessage(this, "Client accepted from " + ip + ":" + port, MessageType.Info); });

                    Thread clientThread = new Thread(new ParameterizedThreadStart(ProcessClientCommunication));

                    ThreadInfo info = new ThreadInfo()
                    {
                        Address = ip + ":" + port,
                        Client = tcpClient,
                        WaitEvent = new ManualResetEvent(false),
                        ClientThread = clientThread
                    };

                    threadHold[ip + ":" + port] = info;
                    clientThread.Start(info);
                }
                catch (Exception)
                {
                    break;
                }
            }
        }

        private void ProcessClientCommunication(object threadInfo)
        {
            ThreadInfo info = (ThreadInfo)threadInfo;
            try
            {
                info.Client.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, true);

                DateTime beginTime = DateTime.Now;
                SocketStates currentState = SocketStates.PendingHeader;

                PacketHeader header = new PacketHeader();
                PacketHeaderData headerData = new PacketHeaderData();
                PacketBeginIterationHeader iterationHeader = new PacketBeginIterationHeader();
                PacketAcnowlege acknowlege = new PacketAcnowlege();

                while (info.Client.Connected && (currentState != SocketStates.CloseConnection))
                {
                    switch (currentState)
                    {
                        case SocketStates.PendingHeader:
                            {
                                header = ReceiveHeader(info);
                                switch (header.type)
                                {
                                    case PacketHeaderType.InitializationHeader:
                                        currentState = SocketStates.PendingHeaderData;
                                        break;

                                    case PacketHeaderType.InitializationData:
                                        break;

                                    case PacketHeaderType.EndOfTest:
                                        {
                                            TimeSpan resultTime = DateTime.Now - beginTime;
                                            Invoke((Action)delegate() { AddMessage(info, "Test acomplished, processing time is " + FormatTime(resultTime) + ".", MessageType.Completed); });
                                            currentState = SocketStates.CloseConnection;
                                        }
                                        break;

                                    case PacketHeaderType.StopIterations:
                                        currentState = SocketStates.CloseConnection;
                                        break;

                                    case PacketHeaderType.NextIteration:
                                        currentState = SocketStates.PendingIterationBegin;
                                        break;
                                }
                            }
                            break;

                        case SocketStates.PendingHeaderData:
                            {
                                headerData = ReceiveHeaderData(info, header);
                                currentState = SocketStates.PendingIterationBegin;
                            }
                            break;

                        case SocketStates.PendingIterationBegin:
                            {
                                iterationHeader = ReceiveIteratioHeader(info);

                                switch (iterationHeader.direction)
                                {
                                    case TransferDirection.Upload:
                                        currentState = SocketStates.ReceiveData;
                                        break;

                                    case TransferDirection.Download:
                                        currentState = SocketStates.SendData;
                                        break;

                                    case TransferDirection.Unknown:
                                        currentState = SocketStates.CloseConnection;
                                        break;
                                }
                            }
                            break;

                        case SocketStates.ReceiveData:
                            {
                                ReceiveData(info, headerData, ref acknowlege);
                                currentState = SocketStates.SendAck;
                            }
                            break;

                        case SocketStates.SendData:
                            {
                                SendData(info, headerData);
                                currentState = SocketStates.PendingHeader;
                            }
                            break;

                        case SocketStates.SendAck:
                            {
                                SendAcknowlegment(info, acknowlege);
                                currentState = SocketStates.PendingHeader;
                            }
                            break;
                    }
                }

                //{
                //    TimeSpan resultTime = DateTime.Now - beginTime;
                //    this.Invoke((Action)delegate() { AddMessage(info, "All cycles acomplished, processing time is " + FormatTime(resultTime) + ".", MessageType.Completed); });
                //}
            }
            catch (Exception ex)
            {
                this.BeginInvoke((Action)delegate() { AddMessage(this, ex.Message, MessageType.Error); });
            }
            finally
            {
                info.Client.Client.Disconnect(false);
                info.Client.Close();
            }
        }

        private void SendData(ThreadInfo info, PacketHeaderData headerData)
        {
            try
            {
                NetworkStream clientStream = info.Client.GetStream();
                DateTime beginTime = DateTime.Now;

                int bufferSize = headerData.streamSize < headerData.bufferSize ? headerData.streamSize : headerData.bufferSize;
                int totalStreamSent = 0;
                int messageRow = -1;
                string sendingMessage = "";

                string ip = ((IPEndPoint)info.Client.Client.RemoteEndPoint).Address.ToString();
                string port = ((IPEndPoint)info.Client.Client.RemoteEndPoint).Port.ToString();

                byte[] pattern = new byte[256];
                byte[] buffer = new byte[headerData.bufferSize];

                int index = 0;
                for (int i = 0; i < headerData.bufferSize; i++)
                {
                    buffer[i] = (byte)index;
                    if (++index > 255)
                        index = 0;
                }

                do
                {
                    clientStream.Write(buffer, 0, bufferSize);
                    totalStreamSent += headerData.bufferSize;

                    sendingMessage = "Sending " + totalStreamSent.ToString() + " of " + headerData.streamSize.ToString() + " bytes to " + ip + ":" + port;
                    if (messageRow == -1)
                        this.Invoke((Action)delegate() { messageRow = AddMessage(info, sendingMessage, MessageType.Communication); });
                    else
                        this.Invoke((Action)delegate() { ReplaceMessage(info, sendingMessage, MessageType.Communication); });
                }
                while ((totalStreamSent < headerData.streamSize) && !info.WaitEvent.WaitOne(0));

                if (totalStreamSent == headerData.streamSize)
                {
                    TimeSpan resultTime = DateTime.Now - beginTime;

                    this.Invoke((Action)delegate() { AddMessage(this, "Data send completed. (" + FormatTime(resultTime) + ") Average speed: " + CalculateSpeed(totalStreamSent, resultTime), MessageType.Completed); });
                    this.BeginInvoke((Action)delegate() { AddMessage(this, "Totally sent " + totalStreamSent.ToString() + " bytes.", MessageType.Info); });
                }
                else
                {
                    this.Invoke((Action)delegate() { AddMessage(this, "File send failed. (" + totalStreamSent.ToString() + " of " + headerData.streamSize.ToString() + ")", MessageType.Error); });
                    return;
                }

                if (info.WaitEvent.WaitOne(0))
                {
                    this.BeginInvoke((Action)delegate() { AddMessage(this, "Thread stopped by server", MessageType.Info); });
                    return;
                }
            }
            catch (Exception ex)
            {
                this.BeginInvoke((Action)delegate() { AddMessage(this, ex.Message, MessageType.Error); });
            }
        }

        private void SendAcknowlegment(ThreadInfo info, PacketAcnowlege ack)
        {
            NetworkStream clientStream = info.Client.GetStream();

            int size = Marshal.SizeOf(typeof(PacketAcnowlege));
            byte[] message = new byte[size];

            unsafe
            {
                fixed (byte* pMsg = message)
                {
                    *((PacketAcnowlege*)pMsg) = ack;
                }
            }

            clientStream.Write(message, 0, size);
        }

        private void ApplyBytes(ref byte[] dest, byte[] src, int startFrom)
        {
            for (int i = 0; i < src.Length; i++)
            {
                dest[startFrom + i] = src[i];
            }
        }

        private string CalculateSpeed(int size, TimeSpan time)
        {
            string speed;
            double dSpeed = ((size / time.TotalSeconds) / (1024 * 1024)) * 8;
            if (dSpeed >= 1)
            {
                dSpeed = Math.Round(dSpeed, 1);
                speed = dSpeed.ToString() + " Mbps";
            }
            else
            {
                dSpeed = ((size / time.TotalSeconds) / 1024) * 8;
                dSpeed = Math.Round(dSpeed, 0);
                speed = dSpeed.ToString() + " Kbps";
            }

            return speed;
        }

        private void ReceiveData(ThreadInfo info, PacketHeaderData headerData, ref PacketAcnowlege ack)
        {
            NetworkStream clientStream = info.Client.GetStream();
            int bufferSize = headerData.streamSize < headerData.bufferSize ? headerData.streamSize : headerData.bufferSize;
            int totalStreamRead = 0;
            bool bRes = true;
            int messageRow = -1;

            DateTime beginTime = DateTime.Now;

            string receivingMessage = "";
            string ip = ((IPEndPoint)info.Client.Client.RemoteEndPoint).Address.ToString();
            string port = ((IPEndPoint)info.Client.Client.RemoteEndPoint).Port.ToString();
            Stopwatch stopper = new Stopwatch();

            do
            {
                byte[] data = new byte[bufferSize];
                int bytesRead = 0;
                int totalRead = 0;

                if (clientStream.CanRead)
                {
                    do
                    {
                        long len = data.Length - totalRead;
                        byte[] temp = new byte[len];
                        bytesRead = clientStream.Read(temp, 0, temp.Length);

                        ApplyBytes(ref data, temp, totalRead);
                        totalRead += bytesRead;
                    }
                    while (clientStream.DataAvailable && !info.WaitEvent.WaitOne(0) && (totalRead != bufferSize));

                }

                if (totalRead == 0)
                {
                    if (stopper.ElapsedMilliseconds > 5000)
                    {
                        stopper.Stop();
                        return;
                    }

                    if (!stopper.IsRunning)
                        stopper.Start();
                }
                else
                {
                    stopper.Stop();
                    stopper.Reset();
                }

                totalStreamRead += totalRead;
                bRes = !CheckPattern(data, totalRead) ? false : bRes;

                receivingMessage = "Receiving " + totalStreamRead.ToString() + " of " + headerData.streamSize.ToString() + " bytes from " + ip + ":" + port;
                if (messageRow == -1)
                    this.Invoke((Action)delegate() { messageRow = AddMessage(info, receivingMessage, MessageType.Communication); });
                else
                    this.Invoke((Action)delegate() { ReplaceMessage(info, receivingMessage, MessageType.Communication); });
            }
            while ((totalStreamRead < headerData.streamSize) && !info.WaitEvent.WaitOne(0));

            if (totalStreamRead == headerData.streamSize)
            {
                TimeSpan resultTime = DateTime.Now - beginTime;

                this.Invoke((Action)delegate() { AddMessage(this, "Data receive completed. (" + FormatTime(resultTime) + ") Average speed: " + CalculateSpeed(totalStreamRead, resultTime), MessageType.Completed); });
                this.BeginInvoke((Action)delegate() { AddMessage(this, "Totally received " + totalStreamRead.ToString() + " bytes.", MessageType.Info); });
            }
            else
            {
                this.Invoke((Action)delegate() { AddMessage(this, "File receive failed. (" + totalStreamRead.ToString() + " of " + headerData.streamSize.ToString() + ")", MessageType.Error); });
                return;
            }

            if (info.WaitEvent.WaitOne(0))
            {
                this.BeginInvoke((Action)delegate() { AddMessage(this, "Thread stopped by server", MessageType.Info); });
                return;
            }

            ack.ack = bRes ? 0 : 1;
            ack.received = totalStreamRead;
        }

        private bool CheckPattern(byte[] data, int len)
        {
            for (int i = 1; i < len; i++)
            {
                int temp = data[i] - data[i - 1];
                if ((temp != 1) && (temp != -255))
                    return false;
            }

            return true;
        }

        private PacketBeginIterationHeader ReceiveIteratioHeader(ThreadInfo info)
        {
            NetworkStream clientStream = info.Client.GetStream();

            PacketBeginIterationHeader iteratioHeader;
            byte[] data = new byte[Marshal.SizeOf(typeof(PacketBeginIterationHeader))];
            int headerSize = data.Length;
            int bytesRead = 0;
            int totalRead = 0;

            Stopwatch stopper = new Stopwatch();
            stopper.Start();
            while ((totalRead != headerSize) && !info.WaitEvent.WaitOne(0) && stopper.ElapsedMilliseconds < 5000)
            {
                byte[] temp = new byte[data.Length - totalRead];
                bytesRead = clientStream.Read(temp, 0, headerSize - totalRead);

                ApplyBytes(ref data, temp, totalRead);
                totalRead += bytesRead;
            }

            stopper.Stop();
            if (stopper.ElapsedMilliseconds >= 5000)
            {
                return new PacketBeginIterationHeader() { direction = TransferDirection.Unknown };
            }

            unsafe
            {
                fixed (byte* pBuff = data)
                {
                    iteratioHeader = *((PacketBeginIterationHeader*)pBuff);
                }
            }

            return iteratioHeader;
        }

        private PacketHeaderData ReceiveHeaderData(ThreadInfo info, PacketHeader header)
        {
            NetworkStream clientStream = info.Client.GetStream();

            PacketHeaderData headerData;
            byte[] data = new byte[header.size];
            int headerSize = data.Length;
            int bytesRead = 0;
            int totalRead = 0;

            while ((totalRead != headerSize) && !info.WaitEvent.WaitOne(0))
            {
                byte[] temp = new byte[data.Length - totalRead];
                bytesRead = clientStream.Read(temp, 0, headerSize - totalRead);

                ApplyBytes(ref data, temp, totalRead);
                totalRead += bytesRead;
            }

            unsafe
            {
                fixed (byte* pBuff = data)
                {
                    headerData = *((PacketHeaderData*)pBuff);
                }
            }

            return headerData;
        }

        private PacketHeader ReceiveHeader(ThreadInfo info)
        {
            NetworkStream clientStream = info.Client.GetStream();

            PacketHeader header = new PacketHeader();
            byte[] data = new byte[Marshal.SizeOf(typeof(PacketHeader))];
            int headerSize = data.Length;
            int bytesRead = 0;
            int totalRead = 0;

            while ((totalRead != headerSize) && !info.WaitEvent.WaitOne(0))
            {
                byte[] temp = new byte[data.Length - totalRead];
                bytesRead = clientStream.Read(temp, 0, headerSize - totalRead);

                ApplyBytes(ref data, temp, totalRead);
                totalRead += bytesRead;
            }

            unsafe
            {
                fixed (byte* pBuff = data)
                {
                    header = *((PacketHeader*)pBuff);
                }
            }

            return header;
        }

        //private void ProcessClientCommunication(object threadInfo)
        //{
        //    ThreadInfo info = (ThreadInfo)threadInfo;
        //    try
        //    {
        //        info.Client.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, true);

        //        int numOfCycles;
        //        if (!SendNumOfCyles(info, out numOfCycles))
        //            return;

        //        DateTime beginTime = DateTime.Now;
        //        for (int cycle = 0; cycle < numOfCycles; cycle++)
        //        {
        //            if (numOfCycles > 1)
        //                this.Invoke((Action)delegate() { AddMessage(info, "Proceeding to cycle #" + (cycle + 1).ToString(), MessageType.Info); });

        //            if (!SendFile(info))
        //                return;

        //            ReceiveFile(info);
        //        }

        //        if (numOfCycles > 1)
        //        {
        //            TimeSpan resultTime = DateTime.Now - beginTime;
        //            this.Invoke((Action)delegate() { AddMessage(info, "All cycles acomplished, processing time is " + FormatTime(resultTime) + ".", MessageType.Completed); });
        //        }
        //    }
        //    finally
        //    {
        //        info.Client.Client.Disconnect(false);
        //        info.Client.Close();
        //    }
        //}

        private bool SendNumOfCyles(ThreadInfo info, out int numberOfCycles)
        {
            #region Variable definitions
            string ip = ((IPEndPoint)info.Client.Client.RemoteEndPoint).Address.ToString();
            string port = ((IPEndPoint)info.Client.Client.RemoteEndPoint).Port.ToString();
            string strNumOfCycles = "";
            NetworkStream clientStream = info.Client.GetStream();
            byte[] numOfCycles = new byte[20];
            #endregion

            try
            {
                this.Invoke((Action)delegate() { AddMessage(info, "Arranging " + strNumOfCycles + " cycles with " + ip + ":" + port, MessageType.Communication); });

                char[] chNumOfCycles = strNumOfCycles.ToCharArray();
                for (int i = 0; i < chNumOfCycles.Length; i++)
                {
                    numOfCycles[i] = Convert.ToByte(chNumOfCycles[i]);
                }

                clientStream.Write(numOfCycles, 0, numOfCycles.Length);
                numberOfCycles = int.Parse(strNumOfCycles);
            }
            catch (Exception ex)
            {
                this.BeginInvoke((Action)delegate() { AddMessage(this, ex.Message, MessageType.Error); });
                numberOfCycles = 0;
                return false;
            }
            return true;
        }

        private bool SendFile(ThreadInfo info)
        {
            #region Variable definitions
            string ip = ((IPEndPoint)info.Client.Client.RemoteEndPoint).Address.ToString();
            string port = ((IPEndPoint)info.Client.Client.RemoteEndPoint).Port.ToString();
            string sendingMessage = "";

            int messageRow = -1;
            int bytesRead = 0;
            int totalSent = 0;

            DateTime beginTime = DateTime.Now;

            #endregion

            try
            {
                if (!File.Exists(fileToSendFullName))
                {
                    AddMessage(info, "File " + fileToSendFullName + " is not found.", MessageType.Error);
                    return false;
                }

                NetworkStream clientStream = info.Client.GetStream();
                using (FileStream fileStream = new FileStream(fileToSendFullName, FileMode.Open, FileAccess.Read, FileShare.Read))
                {
                    using (BinaryReader reader = new BinaryReader(fileStream))
                    {
                        #region Send file size

                        byte[] fileSize = new byte[20];
                        char[] chFileSize = fileStream.Length.ToString().ToCharArray();

                        for (int i = 0; i < chFileSize.Length; i++)
                        {
                            fileSize[i] = Convert.ToByte(chFileSize[i]);
                        }

                        clientStream.Write(fileSize, 0, fileSize.Length);

                        #endregion

                        do
                        {
                            byte[] message = new byte[info.Client.SendBufferSize];
                            bytesRead = fileStream.Read(message, 0, message.Length);

                            if (bytesRead != 0)
                            {
                                clientStream.Write(message, 0, bytesRead);

                                totalSent += bytesRead;

                                sendingMessage = "Sending " + totalSent.ToString() + " of " + fileStream.Length.ToString() + " bytes to " + ip + ":" + port;
                                if (messageRow == -1)
                                    this.Invoke((Action)delegate() { messageRow = AddMessage(info, sendingMessage, MessageType.Communication); });
                                else
                                    this.Invoke((Action)delegate() { ReplaceMessage(info, sendingMessage, MessageType.Communication); });
                            }
                        }
                        while ((bytesRead != 0) && !info.WaitEvent.WaitOne(0));

                        if (totalSent == fileStream.Length)
                        {
                            TimeSpan resultTime = DateTime.Now - beginTime;

                            this.Invoke((Action)delegate() { AddMessage(this, "File send completed. (" + FormatTime(resultTime) + ")", MessageType.Completed); });
                            this.BeginInvoke((Action)delegate() { AddMessage(this, "Totally sent " + totalSent.ToString() + " bytes.", MessageType.Info); });
                        }
                        else
                        {
                            this.Invoke((Action)delegate() { AddMessage(this, "File send failed. (" + totalSent.ToString() + " of " + fileStream.Length.ToString() + ")", MessageType.Error); });
                            return false;
                        }

                        if (info.WaitEvent.WaitOne(0))
                        {
                            this.BeginInvoke((Action)delegate() { AddMessage(this, "Thread stopped by server", MessageType.Info); });
                            return false;
                        }

                        reader.Close();
                    }
                    fileStream.Close();
                }
            }
            catch (Exception ex)
            {
                this.BeginInvoke((Action)delegate() { AddMessage(this, ex.Message, MessageType.Error); });
                return false;
            }

            return true;
        }

        private string FormatTime(TimeSpan resultTime)
        {
            return resultTime.Hours.ToString("00") + ":" + resultTime.Minutes.ToString("00") + ":" + resultTime.Seconds.ToString("00");
        }

        //private bool ReceiveFile(ThreadInfo info)
        //{
        //    #region Variable definitions
        //    bool bSizeFound = false;

        //    Int32 receivingFileSize = 0;
        //    Int32 totalReceived = 0;
        //    int messageRow = -1;

        //    string receivingMessage = "";
        //    string ip = ((IPEndPoint)info.Client.Client.RemoteEndPoint).Address.ToString();
        //    string port = ((IPEndPoint)info.Client.Client.RemoteEndPoint).Port.ToString();
        //    string receivedFileName = @"c:\tmp\received " + DateTime.Now.ToString().Replace("/", "-").Replace(":", ".");

        //    receivedFileName += " " + ip + "-" + port + ".txt";

        //    #endregion

        //    NetworkStream clientStream = info.Client.GetStream();
        //    info.Client.NoDelay = true;
        //    info.Client.ReceiveTimeout = 20000;
        //    int bytesRead = -1;
        //    DateTime beginTime = DateTime.Now;

        //    List<byte[]> messages = new List<byte[]>();
        //    do
        //    {
        //        if (!bSizeFound)
        //        {
        //            #region Get file size
        //            try
        //            {
        //                int temp = info.Client.ReceiveBufferSize;
        //                info.Client.ReceiveBufferSize = 20;

        //                byte[] fileSize = new byte[20];
        //                bytesRead = clientStream.Read(fileSize, 0, info.Client.ReceiveBufferSize);
        //                info.Client.ReceiveBufferSize = temp;

        //                string strFileSize = "";
        //                foreach (var item in fileSize)
        //                {
        //                    strFileSize += Convert.ToChar(item);
        //                }

        //                receivingFileSize = Convert.ToInt32(strFileSize);
        //                this.Invoke((Action)delegate() { AddMessage(this, "Preparing to receive " + receivingFileSize.ToString() + " bytes.", MessageType.Info); });

        //                bSizeFound = true;
        //            }
        //            catch (Exception ex)
        //            {
        //                this.BeginInvoke((Action)delegate() { AddMessage(this, ex.Message, MessageType.Error); });
        //                break;
        //            }
        //            #endregion
        //        }

        //        if (totalReceived == receivingFileSize)
        //        {
        //            break;
        //        }

        //        try
        //        {
        //            byte[] message = new byte[info.Client.ReceiveBufferSize];

        //            bytesRead = clientStream.Read(message, 0, info.Client.ReceiveBufferSize);
        //            if (bytesRead != 0)
        //            {
        //                totalReceived += bytesRead;
        //                messages.Add(message.Take(bytesRead).ToArray());

        //                receivingMessage = "Receiving " + totalReceived.ToString() + " of " + receivingFileSize.ToString() + " bytes from " + ip + ":" + port;
        //                if (messageRow == -1)
        //                    this.Invoke((Action)delegate() { messageRow = AddMessage(info, receivingMessage, MessageType.Communication); });
        //                else
        //                    this.Invoke((Action)delegate() { ReplaceMessage(info, receivingMessage, MessageType.Communication); });
        //            }
        //        }
        //        catch (Exception ex)
        //        {
        //            this.BeginInvoke((Action)delegate() { AddMessage(this, ex.Message, MessageType.Error); });
        //            break;
        //        }
        //    }
        //    while ((bytesRead != 0) && !info.WaitEvent.WaitOne(0));

        //    if (totalReceived == receivingFileSize)
        //    {
        //        TimeSpan resultTime = DateTime.Now - beginTime;

        //        this.Invoke((Action)delegate() { AddMessage(this, "File receive completed. (" + FormatTime(resultTime) + ")", MessageType.Completed); });
        //        this.BeginInvoke((Action)delegate() { AddMessage(this, "Totally received " + totalReceived.ToString() + " bytes.", MessageType.Info); });

        //        if (messages.Count > 0)
        //        {
        //            using (FileStream fileStream = new FileStream(receivedFileName, FileMode.CreateNew))
        //            {
        //                using (BinaryWriter writer = new BinaryWriter(fileStream))
        //                {
        //                    this.Invoke((Action)delegate() { AddMessage(this, "Flushing file to disk (" + receivedFileName + ")", MessageType.Written); });

        //                    foreach (var item in messages)
        //                    {
        //                        writer.Write(item, 0, item.Length);
        //                    }
        //                    writer.Close();
        //                }
        //                fileStream.Close();
        //            }
        //        }
        //    }
        //    else
        //    {
        //        this.Invoke((Action)delegate() { AddMessage(this, "File receiving failed. (" + totalReceived.ToString() + " of " + receivingFileSize.ToString() + ")", MessageType.Error); });
        //        return false;
        //    }

        //    if (info.WaitEvent.WaitOne(0))
        //    {
        //        this.BeginInvoke((Action)delegate() { AddMessage(this, "Thread stopped by server", MessageType.Info); });
        //        return false;
        //    }

        //    return true;
        //}

        private bool ReceiveFile(ThreadInfo info)
        {
            #region Variable definitions
            bool bSizeFound = false;

            Int32 receivingFileSize = 0;
            Int32 totalReceived = 0;
            int messageRow = -1;

            string receivingMessage = "";
            string ip = ((IPEndPoint)info.Client.Client.RemoteEndPoint).Address.ToString();
            string port = ((IPEndPoint)info.Client.Client.RemoteEndPoint).Port.ToString();
            string receivedFileName = @"c:\tmp\received " + DateTime.Now.ToString().Replace("/", "-").Replace(":", ".");

            receivedFileName += " " + ip + "-" + port + ".txt";

            #endregion

            NetworkStream clientStream = info.Client.GetStream();
            info.Client.NoDelay = true;
            info.Client.ReceiveTimeout = 20000;
            int bytesRead = -1;
            DateTime beginTime = DateTime.Now;

            List<byte[]> messages = new List<byte[]>();
            do
            {
                if (!bSizeFound)
                {
                    #region Get file size
                    try
                    {
                        int temp = info.Client.ReceiveBufferSize;
                        info.Client.ReceiveBufferSize = 20;

                        byte[] fileSize = new byte[20];
                        bytesRead = clientStream.Read(fileSize, 0, info.Client.ReceiveBufferSize);
                        info.Client.ReceiveBufferSize = temp;

                        string strFileSize = "";
                        foreach (var item in fileSize)
                        {
                            strFileSize += Convert.ToChar(item);
                        }

                        receivingFileSize = Convert.ToInt32(strFileSize);
                        this.Invoke((Action)delegate() { AddMessage(this, "Preparing to receive " + receivingFileSize.ToString() + " bytes.", MessageType.Info); });

                        bSizeFound = true;
                    }
                    catch (Exception ex)
                    {
                        this.BeginInvoke((Action)delegate() { AddMessage(this, ex.Message, MessageType.Error); });
                        break;
                    }
                    #endregion
                }

                if (totalReceived == receivingFileSize)
                {
                    break;
                }

                try
                {
                    byte[] message = new byte[info.Client.ReceiveBufferSize];

                    bytesRead = clientStream.Read(message, 0, info.Client.ReceiveBufferSize);
                    if (bytesRead != 0)
                    {
                        totalReceived += bytesRead;
                        messages.Add(message.Take(bytesRead).ToArray());

                        receivingMessage = "Receiving " + totalReceived.ToString() + " of " + receivingFileSize.ToString() + " bytes from " + ip + ":" + port;
                        if (messageRow == -1)
                            this.Invoke((Action)delegate() { messageRow = AddMessage(info, receivingMessage, MessageType.Communication); });
                        else
                            this.Invoke((Action)delegate() { ReplaceMessage(info, receivingMessage, MessageType.Communication); });
                    }
                }
                catch (Exception ex)
                {
                    this.BeginInvoke((Action)delegate() { AddMessage(this, ex.Message, MessageType.Error); });
                    break;
                }
            }
            while ((bytesRead != 0) && !info.WaitEvent.WaitOne(0));

            if (totalReceived == receivingFileSize)
            {
                TimeSpan resultTime = DateTime.Now - beginTime;

                this.Invoke((Action)delegate() { AddMessage(this, "File receive completed. (" + FormatTime(resultTime) + ")", MessageType.Completed); });
                this.BeginInvoke((Action)delegate() { AddMessage(this, "Totally received " + totalReceived.ToString() + " bytes.", MessageType.Info); });

                if (messages.Count > 0)
                {
                    using (FileStream fileStream = new FileStream(receivedFileName, FileMode.CreateNew))
                    {
                        using (BinaryWriter writer = new BinaryWriter(fileStream))
                        {
                            this.Invoke((Action)delegate() { AddMessage(this, "Flushing file to disk (" + receivedFileName + ")", MessageType.Written); });

                            foreach (var item in messages)
                            {
                                writer.Write(item, 0, item.Length);
                            }
                            writer.Close();
                        }
                        fileStream.Close();
                    }
                }
            }
            else
            {
                this.Invoke((Action)delegate() { AddMessage(this, "File receiving failed. (" + totalReceived.ToString() + " of " + receivingFileSize.ToString() + ")", MessageType.Error); });
                return false;
            }

            if (info.WaitEvent.WaitOne(0))
            {
                this.BeginInvoke((Action)delegate() { AddMessage(this, "Thread stopped by server", MessageType.Info); });
                return false;
            }

            return true;
        }

        private void buttonClearLog_Click(object sender, EventArgs e)
        {
            listMessages.Items.Clear();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            bool createdNew = false;
            serverMutex = new Mutex(true, "TCP/IP Server", out createdNew);
            if (!createdNew)
            {
                MessageBox.Show("Server is already running");
                Application.Exit();
            }

            Text += " v." + System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.ToString();

            ListViewHelper.EnableDoubleBuffer(listMessages);
            checkAlwaysOnTop.Checked = true;
            Application.ApplicationExit += new EventHandler(Application_ApplicationExit);

            #region Event Viewer Log
            if (!EventLog.SourceExists(logSource))
                EventLog.CreateEventSource(logSource, "QAA Server");
            #endregion
        }

        void Application_ApplicationExit(object sender, EventArgs e)
        {
            StopServer();
        }

        private void MainForm_Resize(object sender, EventArgs e)
        {
            if (WindowState == FormWindowState.Minimized)
            {
                Hide();
                notifyTrayIcon.ShowBalloonTip(1000, "Information", "Server is still running", ToolTipIcon.Info);
            };
        }

        private void notifyTrayIcon_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            ShowMainWindow(!Visible);
        }

        private void ShowMainWindow(bool show)
        {
            if (!show)
            {
                WindowState = FormWindowState.Minimized;
                Hide();
                notifyTrayIcon.ShowBalloonTip(1000, "Information", "Server is still running", ToolTipIcon.Info);
            }
            else
            {
                Show();
                WindowState = FormWindowState.Normal;
            }
        }

        private void buttonExit_Click(object sender, EventArgs e)
        {
            StopServer();
            Application.Exit();
        }

        private void notifyTrayIcon_BalloonTipClicked(object sender, EventArgs e)
        {
            ShowMainWindow(true);
        }

        private void toolStripMenuTrayExit_Click(object sender, EventArgs e)
        {
            StopServer();
            Application.Exit();
        }

        private void checkAlwaysOnTop_CheckedChanged(object sender, EventArgs e)
        {
            this.TopMost = checkAlwaysOnTop.Checked;
        }

        private void timerStartServer_Tick(object sender, EventArgs e)
        {
            timerStartServer.Enabled = false;
            WindowState = FormWindowState.Minimized;
            StartServer();
        }

        private void toolStripMenuRestore_Click(object sender, EventArgs e)
        {
            ShowMainWindow(true);
        }

        private void toolStripMenuLogStop_Click(object sender, EventArgs e)
        {
            try
            {
                string address = listMessages.SelectedItems[0].SubItems[2].Text;
                if (threadHold.ContainsKey(address))
                {
                    ThreadInfo info = threadHold[address];
                    info.WaitEvent.Set();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void contextMenuLog_Opening(object sender, CancelEventArgs e)
        {
            if (listMessages.SelectedItems == null)
            {
                e.Cancel = true;
                return;
            }

            try
            {
                int count = listMessages.SelectedItems[0].SubItems.Count;
                if (count > 2)
                {
                    string address = listMessages.SelectedItems[0].SubItems[2].Text;
                    e.Cancel = !threadHold.ContainsKey(address);
                }
                else
                {
                    e.Cancel = true;
                }
            }
            catch (Exception)
            {
                e.Cancel = true;
                return;
            }
        }

        private void timerCheckThreads_Tick(object sender, EventArgs e)
        {
            List<string> toDelete = new List<string>();

            foreach (var info in threadHold)
            {
                if (!info.Value.ClientThread.IsAlive)
                {
                    toDelete.Add(info.Key);
                }
            }

            foreach (var key in toDelete)
            {
                threadHold.Remove(key);
            }
        }
    }
}
