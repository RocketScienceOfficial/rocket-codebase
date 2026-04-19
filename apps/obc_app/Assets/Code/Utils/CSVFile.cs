using System;
using System.IO;
using System.Text;
using UnityEngine;

public class CSVFile
{
    private readonly StringBuilder _csvBuilder = new();
    private StreamWriter _writer;

    public void Open(string path)
    {
        EnsureDirectoryExists(path);

        _writer = new StreamWriter(path);

        Debug.Log($"{path} has been opened!");
    }

    public void Close()
    {
        if (_writer != null)
        {
            _csvBuilder.Clear();
            _writer.Close();
            _writer = null;

            Debug.Log($"File has been closed!");
        }
    }

    public void WriteString(string str)
    {
        _csvBuilder.Append(str);
        _csvBuilder.Append(',');
    }

    public void WriteFileValue<T>(T value) where T : IComparable, IFormattable, IConvertible, IComparable<T>, IEquatable<T>
    {
        _csvBuilder.Append(value.ToString().Replace(',', '.'));
        _csvBuilder.Append(',');
    }

    public void EndLine()
    {
        _writer.WriteLine(_csvBuilder);
        _csvBuilder.Clear();
    }

    private void EnsureDirectoryExists(string filePath)
    {
        var fi = new FileInfo(filePath);

        if (!fi.Directory.Exists)
        {
            Directory.CreateDirectory(fi.DirectoryName);
        }
    }
}