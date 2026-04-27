int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("Usage: lzma_enc <input> <output>\n");
        return 1;
    }

    // --- Read input ---
    FILE* fin = fopen(argv[1], "rb");
    if (!fin)
    {
        printf("Cannot open input: %s\n", argv[1]);
        return 1;
    }

    fseek(fin, 0, SEEK_END);
    size_t inSize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    uint8_t* inBuf = new uint8_t[inSize];
    fread(inBuf, 1, inSize, fin);
    fclose(fin);

    // --- Allocate output ---
    // Worst case: LZMA can expand incompressible data slightly.
    // 13 bytes header + input size + some overhead is safe.
    size_t outSize = inSize + inSize / 10 + 1024;
    uint8_t* outBuf = new uint8_t[outSize];

    // --- Encode ---
    LzmaEncSimple enc(inBuf, inSize, outBuf, outSize);
    bool ok = enc.Encode();

    if (!ok) {
        printf("Encoding failed\n");
        delete[] inBuf;
        delete[] outBuf;
        return 1;
    }

    // --- Write output ---
    size_t compressedSize = enc.GetOutputSize();

    FILE* fout = fopen(argv[2], "wb");
    if (!fout) {
        printf("Cannot open output: %s\n", argv[2]);
        delete[] inBuf;
        delete[] outBuf;
        return 1;
    }

    fwrite(outBuf, 1, compressedSize, fout);
    fclose(fout);

    // --- Report ---
    printf("Input:    %zu bytes\n", inSize);
    printf("Output:   %zu bytes\n", compressedSize);
    printf("Ratio:    %.2f%%\n", (compressedSize * 100.0) / inSize);

    delete[] inBuf;
    delete[] outBuf;
    return 0;
}
