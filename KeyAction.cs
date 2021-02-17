using KeyNavigator.KeyControls;
using KeyNavigator.Properties;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Diagnostics;
using System.DirectoryServices.ActiveDirectory;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using System.Security.Permissions;
using System.Text;
using System.Windows.Forms;

namespace KeyNavigator
{
    //factory design pattern
    //The possible types and properties
    [Serializable]
    public partial class KeyAction
    {
        //https://docs.microsoft.com/en-us/dotnet/standard/serialization/serialization-guidelines
        [Serializable]
        public abstract partial class IAction : ISerializable
        {
            public IAction()
            {
                Id = Guid.NewGuid();
                Name = "";
                Keys = new List<string>();
                KeysActivationCount = new List<int>();
            }
            public IAction(IAction action)
            {
                Id = action.Id;
                Name = action.Name;
                Keys = action.Keys;
                KeysActivationCount = action.KeysActivationCount;
            }
            public IAction(Guid Id, string Name, List<string> Keys, List<int> KeysActivationCount)
            {
                this.Id = Id;
                this.Name = Name;

                //ensure the Keys aren't null
                if (Keys == null)
                    this.Keys = new List<string>();
                else
                    this.Keys = Keys;

                //ensure the activation count isn't able to be null
                if (KeysActivationCount == null)
                    this.KeysActivationCount = new List<int>();
                else
                    this.KeysActivationCount = KeysActivationCount;
            }

            /// <summary>
            /// Structure to deserialize into
            /// </summary>
            /// <param name="info">The Streaming information</param>
            /// <param name="context">The data stream</param>
            public IAction(SerializationInfo info, StreamingContext context)
            {
                try
                {
                    Id = (Guid)info.GetValue("Id", typeof(Guid));
                    Name = (string)info.GetValue("Name", typeof(string));
                    Keys = (List<string>)info.GetValue("Keys", typeof(List<string>));
                    KeysActivationCount = (List<int>)info.GetValue("KeysActivationCount", typeof(List<int>));
                    IsLocked = (bool)info.GetValue("IsLocked", typeof(bool));
                }
                catch (SerializationException)
                {
                    Id = Guid.Empty;
                    Name = "";
                    Keys = new List<string>();
                    KeysActivationCount = new List<int>();
                    IsLocked = false;
                }

                //ensure there is a corresponding KeysActivationCount for each Key
                while (KeysActivationCount.Count < Keys.Count)
                    KeysActivationCount.Add(0);
            }
            [SecurityPermission(SecurityAction.LinkDemand, Flags = SecurityPermissionFlag.SerializationFormatter)]
            public virtual void GetObjectData(SerializationInfo info, StreamingContext context)
            {
                info.AddValue("Id", Id);
                info.AddValue("Name", Name);
                info.AddValue("Keys", Keys);
                info.AddValue("IsLocked", IsLocked);
                info.AddValue("KeysActivationCount", KeysActivationCount);
            }

            [NonSerialized]
            /// <summary>
            /// internal data store to prevent redundant deserialization calls
            /// </summary>
            private static List<IAction> Data = null;

            [NonSerialized]
            /// <summary>
            /// internal data store to prevent redundant deserialization calls
            /// </summary>
            public static List<IAction> RestrictedActions = new List<IAction>();

            public abstract IKeyControl GetView();

            /// <summary>
            /// Auto generated identifier
            /// </summary>
            public Guid Id { get; private set; }

            /// <summary>
            /// The name of the KeyAction
            /// </summary>
            public string Name { get; set; }

            /// <summary>
            /// The keys mapped to the KeyAction
            /// </summary>
            public List<string> Keys { get; set; }

            /// <summary>
            /// If the KAction can be updated
            /// </summary>
            public bool IsLocked { get; set; }

            /// <summary>
            /// Count of times the key activates the action
            /// </summary>
            public List<int> KeysActivationCount { get; set; }
        }

        [Serializable]
        public partial class ActionDefault : IAction
        {
            public override IKeyControl GetView()
            {
                return new Control_ActionDefault(this);
            }

            public ActionDefault()
            {
                SetApplications(new List<string>());
            }
            public ActionDefault(Guid Id, string Name, bool? IsLocked, List<string> Keys, List<int> ActivationCount, List<string> Applications)
                : base(Id, Name, Keys, ActivationCount)
            {
                this.SetApplications(Applications);
            }
            public ActionDefault(Guid Id, string Name, bool? IsLocked, List<string> Keys, List<int> ActivationCount)
                : base(Id, Name, Keys, ActivationCount)
            {
                this.SetApplications(new List<string>());
            }
            public ActionDefault(IAction action)
                : base(action)
            {
                this.SetApplications(new List<string>());
            }

            /// <summary>
            /// Structure to deserialize into
            /// </summary>
            /// <param name="info">The Streaming information</param>
            /// <param name="context">The data stream</param>
            public ActionDefault(SerializationInfo info, StreamingContext context)
                : base(info, context)
            {
                try
                {
                    SetApplications((List<string>)info.GetValue("Applications", typeof(List<string>)));
                }
                catch (SerializationException)
                {
                    SetApplications(new List<string>());
                }
            }
            [SecurityPermission(SecurityAction.LinkDemand, Flags = SecurityPermissionFlag.SerializationFormatter)]
            public override void GetObjectData(SerializationInfo info, StreamingContext context)
            {
                base.GetObjectData(info, context);
                info.AddValue("Applications", GetApplications());
            }

            private List<string> applications;

            /// <summary>
            /// A list of application paths
            /// </summary>
            public List<string> GetApplications()
            {
                return applications;
            }

            /// <summary>
            /// A list of application paths
            /// </summary>
            public void SetApplications(List<string> value)
            {
                applications = value;
            }
        }

        [Serializable]
        public partial class ActionCMD : IAction
        {
            public override IKeyControl GetView()
            {
                return new Control_ActionCMD(this);
            }

            public ActionCMD()
            {
                SetCommand("");
                SetIsHidden(true);
            }
            public ActionCMD(Guid Id, string Name, bool? IsLocked, List<string> Keys, List<int> ActivationCount, string Command, bool IsHidden)
                : base(Id, Name, Keys, ActivationCount)
            {
                this.SetCommand(Command);
                this.SetIsHidden(IsHidden);
            }
            public ActionCMD(Guid Id, string Name, bool? IsLocked, List<string> Keys, List<int> ActivationCount)
                : base(Id, Name, Keys, ActivationCount)
            {
                this.SetCommand("");
                this.SetIsHidden(false);
            }
            public ActionCMD(IAction action)
                : base(action)
            {
                this.SetCommand("");
                this.SetIsHidden(false);
            }
            public ActionCMD(SerializationInfo info, StreamingContext context)
                : base(info, context)
            {
                try
                {
                    SetCommand((string)info.GetValue("Command", typeof(string)));
                    SetIsHidden((bool)info.GetValue("IsHidden", typeof(bool)));
                }
                catch (SerializationException)
                {
                    SetCommand("");
                    SetIsHidden(false);
                }
            }

            [SecurityPermission(SecurityAction.LinkDemand, Flags = SecurityPermissionFlag.SerializationFormatter)]
            public override void GetObjectData(SerializationInfo info, StreamingContext context)
            {
                base.GetObjectData(info, context);
                info.AddValue("Command", GetCommand());
                info.AddValue("IsHidden", GetIsHidden());
            }

            private string command;

            /// <summary>
            /// Possibly multiline Terminal command
            /// </summary>
            public string GetCommand()
            {
                return command;
            }

            /// <summary>
            /// Possibly multiline Terminal command
            /// </summary>
            public void SetCommand(string value)
            {
                command = value;
            }

            private bool isHidden;

            /// <summary>
            /// Whether the command executes with visibility
            /// </summary>
            public bool GetIsHidden()
            {
                return isHidden;
            }

            /// <summary>
            /// Whether the command executes with visibility
            /// </summary>
            public void SetIsHidden(bool value)
            {
                isHidden = value;
            }
        }

        [Serializable]
        public partial class ActionWeb : IAction
        {
            public override IKeyControl GetView()
            {
                return new Control_ActionWeb(this);
            }

            public ActionWeb()
            {
                SetWebsites(new List<string>());
            }
            public ActionWeb(Guid Id, string Name, bool? IsLocked, List<string> Keys, List<int> ActivationCount, List<string> Websites)
                : base(Id, Name, Keys, ActivationCount)
            {
                this.SetWebsites(Websites);
            }
            public ActionWeb(Guid Id, string Name, bool? IsLocked, List<string> Keys, List<int> ActivationCount)
                : base(Id, Name, Keys, ActivationCount)
            {
                this.SetWebsites(new List<string>());
            }
            public ActionWeb(IAction action)
                : base(action)
            {
                this.SetWebsites(new List<string>());
            }
            public ActionWeb(SerializationInfo info, StreamingContext context)
                : base(info, context)
            {
                try
                {
                    SetWebsites((List<string>)info.GetValue("Websites", typeof(List<string>)));
                }
                catch (SerializationException)
                {
                    SetWebsites(new List<string>());
                }
            }
            [SecurityPermission(SecurityAction.LinkDemand, Flags = SecurityPermissionFlag.SerializationFormatter)]
            public override void GetObjectData(SerializationInfo info, StreamingContext context)
            {
                base.GetObjectData(info, context);
                info.AddValue("Websites", GetWebsites());
            }

            private List<string> websites;

            /// <summary>
            /// A list of website URLs
            /// </summary>
            public List<string> GetWebsites()
            {
                return websites;
            }

            /// <summary>
            /// A list of website URLs
            /// </summary>
            public void SetWebsites(List<string> value)
            {
                websites = value;
            }
        }
    }

    //contains all the functions
    public partial class KeyAction
    {
        public abstract partial class IAction
        {
            /// <summary>
            /// Launch the attached method based off the key
            /// </summary>
            /// <returns>A boolean of whether the task executed</returns>
            public abstract bool Execute();

            public static void ExecuteActions(string key)
            {
                //Update KeyAction.cs ActivationCount to only increment the key pressed
                //Data
                Data.ForEach(action =>
                {
                    key = key.ToLower();

                    if (action.Keys.Contains(key))
                    { 
                        action.Execute();
                        action.KeysActivationCount[action.Keys.IndexOf(key)]++;
                        Console.WriteLine(action.Id + "\t" + action.Name);
                        Console.WriteLine(action.KeysActivationCount[0]);
                    }
                });
            }

            /// <summary>
            /// Display the action data in a human readable format
            /// </summary>
            /// <returns>The values from the action in a readable format</returns>
            public override abstract string ToString();
            #region Serialization
            private static string Serialize(object data)
            {
                string serializedData = "";

                IFormatter bf = new BinaryFormatter();
                MemoryStream ms = new MemoryStream();

                //serialize to stream, read from stream, store data
                bf.Serialize(ms, data);
                ms.Position = 0; //once the data is read in the position is the end assuming more wants to be added
                BinaryReader br = new BinaryReader(ms);
                //add to the list which will then be joined by a single character then serialized again
                serializedData = Convert.ToBase64String(br.ReadBytes((int)ms.Length));

                br.Close();
                ms.Close();

                return serializedData;
            }

            private static IAction Deserialize(object data)
            {
                try
                {
                    IFormatter bf = new BinaryFormatter();

                    MemoryStream ms = new MemoryStream(); //create stream the size of the stored contents
                    BinaryWriter bw = new BinaryWriter(ms);

                    byte[] binaryData = Convert.FromBase64String(data.ToString());

                    //write data to stream
                    bw.Write(binaryData, 0, binaryData.Length);
                    bw.Flush(); //flush stream writer after writing, ensure it pushes to ms
                    ms.Position = 0;

                    IAction deserializedData = bf.Deserialize(ms) as IAction;

                    bw.Close();
                    ms.Close();

                    return deserializedData;
                }
                catch (System.Reflection.TargetInvocationException)
                {
                    //Console.WriteLine($"Failure when reading stored data. \n{tie.Source}\n{tie.StackTrace}\n{tie.InnerException}\n{tie.Message}");
                    Console.WriteLine("Failure when reading stored data.");
                }
                catch (System.Runtime.Serialization.SerializationException)
                {
                    Console.WriteLine("Failure to deserialize stored data.");
                }
                catch (System.FormatException)
                {
                    ClearActions();
                    Console.WriteLine("Serialized data was formatted incrorrectly.");
                }
                return null;
            }
            #endregion
            #region ActionManipulation
            /// <summary>
            /// Add a single action to internal storage without serialization of the entire list
            /// </summary>
            /// <param name="action">The action to append to the current serialized string</param>
            public static void AddAction(IAction action)
            {
                if (IAction.Data == null || !IAction.Data.Contains(action))
                {
                    //action as string, append to the current action string
                    var actionResult = ConvertActionToString(action);

                    //add the data to the list or assign the internal storage under the presumption of a new list
                    if (Settings.Default.InternalActions.Length > 0)
                    { 
                        Settings.Default.InternalActions += "|" + actionResult;
                    }
                    else 
                    {
                        Settings.Default.InternalActions += actionResult;
                    }
                    Settings.Default.Save();

                    Views.internalData.Add(action);
                    //Console.WriteLine($"Size: {Settings.Default.InternalActions.Split('|')}\tInternalData: {Settings.Default.InternalActions}");
                }
                else
                {
                    var listResult = IAction.Data.Where(a => !a.Equals(action)).ToList();
                    listResult.Add(action);
                    SaveActions(listResult);
                }
            }

            public static void SaveActions(List<IAction> actionList)
            {
                //no items are in the list that will be saved
                if (actionList.Count == 0)
                {
                    Settings.Default.InternalActions = "";
                }
                else 
                {
                    Settings.Default.InternalActions = ConvertActionsToString(IAction.Data);
                }
                Settings.Default.Save();
            }
            public static string GetSerializedActions() 
            {
                return Settings.Default.InternalActions;
            }
            private static string ConvertActionsToString(List<IAction> actionList) 
            {
                //no items are in the list that will be saved
                string[] serializedActions = new string[actionList.Count];
                for (int i = 0; i < actionList.Count; i++) 
                {
                    //add to the list which will then be joined by a single character then serialized again
                    serializedActions[i] = ConvertActionToString(actionList[i]);
                }

                string newInternalData = serializedActions[0]; //assign new internal data to the first because there is only one item in the list
                if (serializedActions.Length > 1) //join many serialized values together to later be split
                { 
                    newInternalData = string.Join("|", serializedActions);
                }

                return newInternalData;   
            }

            private static string ConvertActionToString(IAction action)
            {
                string type = "0";
                switch (action.GetType().Name)
                {
                    case "ActionDefault":
                        break;
                    case "ActionCMD":
                        type = "1";
                        break;
                    case "ActionWeb":
                        type = "2";
                        break;
                }

                return type + Serialize(action);
            }

            /// <summary>
            /// Load every action except the parameter, 
            ///     save the stored actions as the refined list
            ///     update the view with the new internal storage
            /// </summary>
            /// <param name="action">The action to be removed from the actions</param>
            public static void RemoveAction(IAction action)
            {
                var actionToRemove = KeyAction.IAction.LoadActions().Where(a => a.Equals(action)).First();

                var remainingActions = LoadActions();
                remainingActions.Remove(actionToRemove);
                Console.WriteLine(remainingActions.Count + "...removing action..." + action.ToString());
                SaveActions(remainingActions);
                Views.internalData = remainingActions;
            }
            public static void ClearActions()
            {
                Settings.Default.InternalActions = "";
                Settings.Default.Save();
            }

            public static List<IAction> LoadActions()
            {
                IAction.Data = ConvertDataToArray(Settings.Default.InternalActions);

                if (Control_Settings.embeddedActions != "")
                {
                    IAction.Data.AddRange(ConvertDataToArray(Control_Settings.embeddedActions));
                }

                return IAction.Data;
            }

            private static List<IAction> ConvertDataToArray(string data) 
            {
                List<IAction> actionsList = new List<IAction>();
                if (string.IsNullOrWhiteSpace(data)) 
                {
                    return actionsList;
                }

                //ween out any instances that the item inbetween has been removed but the separator hasn't 
                string[] binaryCollected = Settings.Default.InternalActions.Split(new string[] { "|" }, StringSplitOptions.RemoveEmptyEntries);
                if (IAction.Data != null && Data.Count() == binaryCollected.Count()) //return data without processing to save time on deserialization
                {
                    return IAction.Data;
                }

                for (int i = 0; i < binaryCollected.Length; i++) //process each stored action in the serialized collection
                {
                    char subType = binaryCollected[i][0];
                    string cleanData = binaryCollected[i].Substring(1); //remove the subType 

                    IAction deserializedObject = Deserialize(cleanData);
                    switch (subType) //identifier is stored within the first character of the serialized object
                    {
                        default:
                        case '0':
                            deserializedObject = deserializedObject as ActionDefault;
                            break;
                        case '1':
                            deserializedObject = deserializedObject as ActionCMD;
                            break;
                        case '2':
                            deserializedObject = deserializedObject as ActionWeb;
                            break;
                    }
                    //Console.WriteLine("Type: " + subType + "\t" + deserializedObject + "\t" + cleanData);
                    //Console.WriteLine("Type: " + subType + "\t" + deserializedObject);

                    actionsList.Add(deserializedObject);
                }
                return actionsList;
            }

            /// <summary>
            /// filter out the actions the user shouldn't see
            /// </summary>
            /// <returns></returns>
            public static List<IAction> LoadActionsRestricted()
            {
                RestrictedActions.Clear();

                var allActions = LoadActions();
                
                //restrict based off of accessability
                //flags mean they can, in this case if they can't do that, then limit the actions visible
                //if true then the user CAN view
                if (!Control_Settings.embeddedFlags[1]) 
                {
                    RestrictedActions.AddRange(allActions.Where(a => a is ActionDefault).ToList());
                }
                if (!Control_Settings.embeddedFlags[3])
                {
                    RestrictedActions.AddRange(allActions.Where(a => a is ActionCMD).ToList());
                }
                if (!Control_Settings.embeddedFlags[5])
                {
                    RestrictedActions.AddRange(allActions.Where(a => a is ActionWeb).ToList());
                }

                return allActions.Where(a => !RestrictedActions.Contains(a)).ToList();
            }

            /// <summary>
            /// Compares two IActions only looking at the Id for the action which will always be unique
            /// </summary>
            /// <param name="obj"></param>
            /// <returns></returns>
            public override bool Equals(object obj)
            {
                if (obj == null) 
                {
                    return false;
                }

                if (obj is IAction)
                {
                    if ((obj as IAction).Id == this.Id)
                    { 
                        return true;
                    }    
                    else 
                    {
                        return false;
                    }
                }
                else 
                {
                    return false;
                }
            }
            #endregion
        }

        public partial class ActionCMD : IAction
        {
            /// <summary>
            /// Create a file in the users temp directory, execute then delete
            /// </summary>
            /// <key>The index of the key list which executed this action</key>
            /// <returns></returns>
            public override bool Execute()
            {
                string fileName = "";

                try
                {
                    string localTemp = Path.GetTempPath();
                    //create auto incrementing FileName
                    int fileNum = 0;
                    while (File.Exists(fileName = Path.Combine(localTemp, "KeyNav_CMD_" + fileNum + ".bat")))
                        fileNum++;
                    File.WriteAllText(fileName, GetCommand());

                    //Assign FileName, Visibility and default states
                    ProcessStartInfo psi = new ProcessStartInfo();
                    psi.FileName = fileName;
                    psi.CreateNoWindow = this.GetIsHidden();
                    psi.UseShellExecute = true;
                    psi.LoadUserProfile = true;
                    psi.WindowStyle = ProcessWindowStyle.Normal;
                    Console.WriteLine(fileName);
                    if (this.GetIsHidden())
                        psi.WindowStyle = ProcessWindowStyle.Hidden;

                    //Process commandProcess = 
                    Process.Start(psi);
                    //commandProcess.WaitForExit();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message + "\n" + e.StackTrace + "\n" + e.Source);
                }
                finally
                {
                    //cleanup file created at the beginning of the method
                    if (File.Exists(fileName))
                        File.Delete(fileName); 
                }
                return true;
            }
            public override string ToString()
            {
                return Name + " Keys[" + Keys.Count + "] Act#[" + KeysActivationCount.Count + "] CommandLen[" + command.Length + "]";
                //return Name + " Lock[" + IsLocked + "] Keys[" + Keys.Count + "] Act#[" + KeysActivationCount.Count + "] Visible[" + !GetIsHidden() + "] Command[" + GetCommand() + "] " + Id;
            }
        }

        public partial class ActionDefault : IAction
        {
            public override bool Execute()
            {
                int applicationId = 0;

                //Might not need to load the UserProfile, as a service it should have access to the
                //  users credentials and accessability
                ProcessStartInfo psi = new ProcessStartInfo();
                psi.LoadUserProfile = true;

                foreach (string application in this.GetApplications())
                {
                    applicationId++;
                    try
                    {
                        psi.FileName = application;
                        Process.Start(psi);
                    }
                    catch (System.ComponentModel.Win32Exception)
                    {
                        string title = "URL Execute Error";
                        string message = "Action: " + this.Name + "\tApplicationId:" + applicationId +"\n" +
                            "There was an error when opening " + application + " or it could not be executed.\n" +
                            "Example: " + Path.GetRandomFileName();
                        Form_Display.DisplayError(title, message);
                        return false;
                    }
                }

                return true;
            }

            public override string ToString()
            {
                return Name + " Keys[" + Keys.Count + "] Act#[" + KeysActivationCount.Count + "] Apps#[" + applications.Count + "]";
                //return Name + " Lock[" + IsLocked + "] Keys[" + Keys.Count + "] Act#[" + KeysActivationCount.Count + "] Apps[" + GetApplications().Count + "] " + Id;
            }
        }

        public partial class ActionWeb : IAction
        {
            public override bool Execute()
            {
                ProcessStartInfo psi = new ProcessStartInfo();
                psi.UseShellExecute = true;
                psi.FileName = "cmd.exe";
                psi.CreateNoWindow = true;
                psi.WindowStyle = ProcessWindowStyle.Hidden;

                for (int i = 0; i < GetWebsites().Count; i++)
                {
                    var website = GetWebsites()[i];

                    //if the URL doesn't have the correct sytax then correct the URL
                    if (!website.StartsWith("http://") && !website.StartsWith("https://"))
                    {
                        website = "http://" + website;
                        //reassign the value to reduce future iterations
                        websites[i] = website;
                    }

                    //if the site fails to open
                    if (IsValidUri(website))
                    {
                        psi.Arguments = "/c start " + website;
                        Process.Start(psi);
                    }
                    else
                    {
                        string title = "URL Execute Error";
                        string message = "Action: " + this.Name + "\tWebsiteId:" + i + "\n" +
                            "There was an error with the format of " + website + " or it could not be executed.\n" +
                            "Example: www.google.ca";
                        Form_Display.DisplayError(title, message);

                        return false;
                    }
                }

                return true;
            }

            /// <summary>
            /// Referenced from https://stackoverflow.com/questions/502199/how-to-open-a-web-page-from-my-application
            /// Check for the validity of a pass website
            /// </summary>
            /// <param name="uri">The website to be verified</param>
            /// <returns>Boolean, if the website is valid</returns>
            private bool IsValidUri(string uri)
            {
                //if it isn't formatted properly return false
                if (!Uri.IsWellFormedUriString(uri, UriKind.Absolute))
                    return false;
                Uri tmp;
                //if it fails to create a URI then return false
                if (!Uri.TryCreate(uri, UriKind.Absolute, out tmp))
                    return false;

                //if the website is HTTP or HTTPS
                return tmp.Scheme == Uri.UriSchemeHttp || tmp.Scheme == Uri.UriSchemeHttps;
            }

            public override string ToString()
            {
                return Name + " Keys[" + Keys.Count + "] Act#[" + KeysActivationCount.Count + "] Site#[" + websites.Count + "]";
                //return Name + "Lock[" + IsLocked + "] Keys[" +Keys.Count + "] Act#[" + KeysActivationCount.Count + "] Sites[" + GetWebsites().Count + "] " + Id;
            }
        }
    }

    //contain datatypes localized to the KeyAction
    public partial class KeyAction
    {
        /// <summary>
        /// Interface to be implemented by the controls
        /// </summary>
        public interface IKeyControl
        {
            KeyAction.IAction GetData();
            void SetData(IAction action);
        }

        public static string GetType_String(IAction action) 
        {
            if (action is ActionDefault)
                return "Default";
            if (action is ActionCMD)
                return "CMD";
            if (action is ActionWeb)
                return "Web";
            
            return "IAction";
        }
        public static string GetType_String(int typeNum)
        {
            if (typeNum == 0)
                return "Default";
            if (typeNum == 1)
                return "CMD";
            if (typeNum == 2)
                return "Web";

            return "IAction";
        }
        public static int GetType_Int(IAction action)
        {
            if (action is ActionDefault)
                return 0;
            if (action is ActionCMD)
                return 1;
            if (action is ActionWeb)
                return 2;

            return 3;
        }

        public static string JoinList<T>(List<T> array, char delim)
        {
            return JoinArray<T>(array.ToArray(), delim);
        }
        public static string JoinArray<T>(T[] array, char delim) 
        {
            string joinedString = "";
            foreach (T s in array)
                joinedString += s.ToString() + delim;
            joinedString = joinedString.Trim(delim);
            return joinedString;
        }
    }
}








/*
 public static void SaveActions(List<KeyAction.IAction> localKeyActions)
            {
                //use the already written method for serialization and deserialization
                //  converts from List<Actions> to binary string
                //use reflection to pull out the executable being run, the file which will store
                //  the list, and create a new .exe on the fly with Ilmake
                var actionList = LoadActions();

                if (actionList.Count == 0)
                {
                    Settings.Default["InternalActions"] = localKeyActions;
                }
                else
                {
                    actionList.AddRange(localKeyActions);
                    Settings.Default["InternalActions"] = actionList;
                }
                
                //If the setting doesn't already exist within the application then 
                //  create it otherwise add to the existing list
                //SettingsProperty[] settings = new SettingsProperty[Settings.Default.Properties.Count];
                //Settings.Default.Properties.CopyTo(settings, 0);

                //var storedSettings_IA = settings.Where(s => s.Name == "InternalActions");

                //SettingsProperty internalActions = Settings.Default.Properties["InternalActions"];

                //if (internalActions == null)
                //{
                //    Console.WriteLine("NEW");
                //    SettingsProperty prop = new SettingsProperty("InternalActions");
                //    prop.PropertyType = typeof(List<KeyAction.IAction>);
                //    prop.SerializeAs = SettingsSerializeAs.Binary;

                //    //prop.DefaultValue = new KeyAction.ActionDefault();
                //    prop.IsReadOnly = false;
                //    prop.ThrowOnErrorDeserializing = true;
                //    prop.ThrowOnErrorSerializing = true;

                //    SettingsPropertyValue propVal = new SettingsPropertyValue(prop);
                //    propVal.PropertyValue = localKeyActions;

                //    //Add property and corresponding value
                //    Settings.Default.Properties.Add(prop);
                //    Settings.Default.PropertyValues.Add(propVal);
                //}
                //else
                //{
                //    Console.WriteLine("UPDATE");
                //    List<KeyAction.IAction> actions = LoadActions();
                //    actions.AddRange(localKeyActions);
                //    Settings.Default.PropertyValues["InternalActions"].PropertyValue = actions;
                //}
                
                //this will throw an error on save/read
                //  binary serialization needs to be implemented
                Settings.Default.Save();
            }
 */
