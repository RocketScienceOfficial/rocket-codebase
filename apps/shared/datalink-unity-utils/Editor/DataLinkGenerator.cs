using System;
using System.Diagnostics;
using System.IO;
using UnityEditor;
using UnityEngine;
using Debug = UnityEngine.Debug;

[InitializeOnLoad]
public class DataLinkGenerator
{
    private static readonly string _projectPath;
    private static readonly string _datalinkPath;
    private static readonly string _generatorScript;
    private static readonly string _schemaDir;
    private static readonly string _templatesDir;
    private static readonly string _outputDir;
    private static readonly string _outputFile;

    static DataLinkGenerator()
    {
       _projectPath = Path.GetDirectoryName(Application.dataPath);
       _datalinkPath = Path.Combine(_projectPath, "..", "..", "datalink");
       _generatorScript = Path.Combine(_datalinkPath, "gen.py");
       _schemaDir = Path.Combine(_datalinkPath, "schemas");
       _templatesDir = Path.Combine(_datalinkPath, "templates");
       _outputDir = _datalinkPath;
       _outputFile = Path.Combine(_outputDir, "datalink.cs");

       if (ShouldRebuild())
       {
           RunPythonGenerator();
       }
    }

    [MenuItem("Tools/Force Generate Datalink")]
    public static void ForceGenerate()
    {
       RunPythonGenerator();
    }

    private static bool ShouldRebuild()
    {
       if (!File.Exists(_outputFile))
       {
           return true;
       }

       var maxTime = DateTime.MinValue;

       if (File.Exists(_generatorScript))
       {
           maxTime = File.GetLastWriteTime(_generatorScript);
       }

       var dirsToWatch = new string[] { _schemaDir, _templatesDir };

       foreach (var dir in dirsToWatch)
       {
           if (Directory.Exists(dir))
           {
               var files = Directory.GetFiles(dir, "*.*", SearchOption.AllDirectories);

               foreach (var file in files)
               {
                   var fileTime = File.GetLastWriteTime(file);

                   if (fileTime > maxTime)
                   {
                       maxTime = fileTime;
                   }
               }
           }
       }

       var lastTime = File.GetLastWriteTime(_outputFile);

       return maxTime > lastTime;
    }

    private static void RunPythonGenerator()
    {
       Debug.Log("Running generator...");

       ProcessStartInfo startInfo = new()
       {
           FileName = "python",
           Arguments = $"\"{_generatorScript}\" --lang csharp --outdir \"{_outputDir}\"",
           UseShellExecute = false,
           RedirectStandardOutput = true,
           RedirectStandardError = true,
           CreateNoWindow = true,
           WorkingDirectory = _projectPath
       };

       using var process = Process.Start(startInfo);

       process.WaitForExit();

       var output = process.StandardOutput.ReadToEnd();
       var errors = process.StandardError.ReadToEnd();

       if (process.ExitCode == 0)
       {
           Debug.Log($"Generation successful!\n{output}");

           AssetDatabase.Refresh();
       }
       else
       {
           Debug.LogError($"Generation failed! Exit Code: {process.ExitCode}\n{errors}");
       }
    }
}