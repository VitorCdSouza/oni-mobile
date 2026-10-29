// StreamingAssets on Android, carried inside the APK.
//
// The game reads its YAML, codex, strings and asset bundles with System.IO from
// Application.streamingAssetsPath, which on Android points inside the APK, where
// System.IO cannot read. tools/scripts/make_apk.sh stores those files uncompressed
// under assets/StreamingAssets/ in the APK, and tools/picktypes/PatchAndroid.cs sends
// every read of the path here. The first read copies them out to
// <persistentDataPath>/StreamingAssets, only the files whose size or CRC differs
// from the last copy (listed in .apk-files there), so a reinstall or a new phone
// gets them back by itself and an update copies just what changed.
//
// The FMOD banks are not copied: make_apk.sh puts them at the APK's assets/ root and
// FMOD reads them from there (file:///android_asset). Banks left in the directory by
// the old push_streaming_assets.sh are deleted, ~930 MB.
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using UnityEngine;

namespace OniMobile
{
    public static class StreamingAssets
    {
        const string Prefix = "assets/StreamingAssets/";
        const string ManifestName = ".apk-files";

        static string path;
        static readonly object gate = new object();

        public static string Path()
        {
            if (path != null)
                return path;
            lock (gate)
            {
                if (path == null)
                {
                    if (Platform.IsAndroid)
                    {
                        string dir = Application.persistentDataPath + "/StreamingAssets";
                        try
                        {
                            Extract(Application.dataPath, dir);
                        }
                        catch (Exception e)
                        {
                            Debug.LogWarning("OniMobile StreamingAssets: copy from the APK failed: " + e);
                        }
                        path = dir;
                    }
                    else
                        path = Application.streamingAssetsPath;
                }
            }
            return path;
        }

        struct Entry
        {
            public string Name;
            public uint Crc, Size;
            public long HeaderOffset;
        }

        static void Extract(string apk, string dir)
        {
            DateTime start = DateTime.UtcNow;
            Directory.CreateDirectory(dir);
            string manifestPath = System.IO.Path.Combine(dir, ManifestName);
            var old = ReadManifest(manifestPath);
            var manifest = new StringBuilder();
            int copied = 0, kept = 0;
            long bytes = 0;
            var buffer = new byte[1 << 20];
            using (var zip = new FileStream(apk, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                foreach (Entry entry in ReadCentralDirectory(zip))
                {
                    string relative = entry.Name.Substring(Prefix.Length);
                    string target = System.IO.Path.Combine(dir, relative);
                    string key = entry.Crc.ToString("x8") + " " + entry.Size;
                    manifest.Append(key).Append(' ').Append(relative).Append('\n');
                    string previous;
                    if (old.TryGetValue(relative, out previous) && previous == key
                        && File.Exists(target) && new FileInfo(target).Length == entry.Size)
                    {
                        kept++;
                        continue;
                    }
                    Directory.CreateDirectory(System.IO.Path.GetDirectoryName(target));
                    if (File.Exists(target))
                        File.Delete(target);
                    zip.Position = DataOffset(zip, entry);
                    using (var output = new FileStream(target, FileMode.Create, FileAccess.Write))
                    {
                        long left = entry.Size;
                        while (left > 0)
                        {
                            int read = zip.Read(buffer, 0, (int)Math.Min(buffer.Length, left));
                            if (read <= 0)
                                throw new EndOfStreamException(entry.Name);
                            output.Write(buffer, 0, read);
                            left -= read;
                        }
                    }
                    copied++;
                    bytes += entry.Size;
                }
            }
            File.WriteAllText(manifestPath, manifest.ToString());
            int banks = 0;
            foreach (string bank in Directory.GetFiles(dir, "*.bank"))
            {
                File.Delete(bank);
                banks++;
            }
            Debug.Log("OniMobile StreamingAssets: " + copied + " files copied from the APK (" + (bytes >> 20) + " MB), "
                + kept + " unchanged, " + banks + " old banks deleted, " + (int)(DateTime.UtcNow - start).TotalMilliseconds + " ms");
        }

        static Dictionary<string, string> ReadManifest(string manifestPath)
        {
            var entries = new Dictionary<string, string>();
            if (!File.Exists(manifestPath))
                return entries;
            foreach (string line in File.ReadAllLines(manifestPath))
            {
                // <crc> <size> <path>; the path may hold spaces.
                int first = line.IndexOf(' ');
                int second = first < 0 ? -1 : line.IndexOf(' ', first + 1);
                if (second > 0)
                    entries[line.Substring(second + 1)] = line.Substring(0, second);
            }
            return entries;
        }

        // The APK's entries under Prefix, from the zip central directory. They must be
        // stored (method 0), which make_apk.sh makes sure of with zip -0.
        static List<Entry> ReadCentralDirectory(FileStream zip)
        {
            int tail = (int)Math.Min(zip.Length, 22 + 0xFFFF);
            var end = new byte[tail];
            zip.Position = zip.Length - tail;
            ReadFully(zip, end, tail);
            int eocd = -1;
            for (int i = tail - 22; i >= 0; i--)
                if (U32(end, i) == 0x06054b50)
                {
                    eocd = i;
                    break;
                }
            if (eocd < 0)
                throw new IOException("no end of central directory in " + zip.Name);
            uint size = U32(end, eocd + 12), offset = U32(end, eocd + 16);
            if (offset == 0xFFFFFFFF)
                throw new IOException("zip64 APK, not supported");
            var directory = new byte[size];
            zip.Position = offset;
            ReadFully(zip, directory, (int)size);

            var entries = new List<Entry>();
            for (int at = 0; at + 46 <= directory.Length && U32(directory, at) == 0x02014b50; )
            {
                int method = U16(directory, at + 10);
                int nameLength = U16(directory, at + 28);
                int extraLength = U16(directory, at + 30), commentLength = U16(directory, at + 32);
                string name = Encoding.UTF8.GetString(directory, at + 46, nameLength);
                if (name.StartsWith(Prefix, StringComparison.Ordinal) && !name.EndsWith("/") && name.Length > Prefix.Length)
                {
                    if (method != 0)
                        throw new IOException(name + " is compressed in the APK");
                    entries.Add(new Entry
                    {
                        Name = name,
                        Crc = U32(directory, at + 16),
                        Size = U32(directory, at + 24),
                        HeaderOffset = U32(directory, at + 42),
                    });
                }
                at += 46 + nameLength + extraLength + commentLength;
            }
            return entries;
        }

        // The local header repeats the name and has its own extra field (zipalign pads it).
        static long DataOffset(FileStream zip, Entry entry)
        {
            var header = new byte[30];
            zip.Position = entry.HeaderOffset;
            ReadFully(zip, header, 30);
            if (U32(header, 0) != 0x04034b50)
                throw new IOException("bad local header for " + entry.Name);
            return entry.HeaderOffset + 30 + U16(header, 26) + U16(header, 28);
        }

        static void ReadFully(Stream stream, byte[] buffer, int count)
        {
            for (int done = 0; done < count; )
            {
                int read = stream.Read(buffer, done, count - done);
                if (read <= 0)
                    throw new EndOfStreamException();
                done += read;
            }
        }

        static int U16(byte[] b, int at)
        {
            return b[at] | b[at + 1] << 8;
        }

        static uint U32(byte[] b, int at)
        {
            return (uint)(b[at] | b[at + 1] << 8 | b[at + 2] << 16 | b[at + 3] << 24);
        }
    }
}
