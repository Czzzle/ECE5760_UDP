function createRGB(filename, output_file, max_width, max_height)
%{
This function is to genertae RGB given a figure ".jpg" or ".png".

INPUTS:
    filename: image file name, can be "jpg" or "png"
    max_width: the max_width of the figure we want to display on VGA
    max_height: the max_height of the figure we want to display on VGA
    
    ******** the image will be kept in the % of orignal size *************
%}

% ======== Step 1: check the image is RGB type ======
info = imfinfo(filename);

if strcmp(info.ColorType, 'indexed')
    [X, map] = imread(filename);
    RGB = ind2rgb(X, map);     % change to RGB, the type is double [0,1]
    I = im2uint8(RGB);         % change to uint8 to match 8-bit color
else
    I = imread(filename);      % if it's RGB already, read it. 
end

[orig_x, orig_y, ~] = size(I); % get the original height (# of row), width (# of col) and channel number

x_percent = max_height/orig_x;

y_percent = max_width/orig_y;

percent = min([x_percent,y_percent]); % keep the orignal width/height ratio, but shrink it 

a = imresize(I,percent); 
[height,width, z] = size(a) ;

% output_file = 'image_data.c';
fid = fopen(output_file, 'w');
fprintf(fid, '#define xmax %d\n', width);
fprintf(fid, '#define ymax %d\n', height);
fprintf(fid, 'const uint8_t trophy_data[ymax][xmax] = {\n');

for x=1:height
    fprintf(fid, '    {');
    for y=1:width
   
    r = a(x,y,1);
    g = a(x,y,2);
    b = a(x,y,3);
    
    
    % RED 3 bits
    if r < 32
        red = 1;
    elseif r < 64
        red = 2;
    elseif r < 96
        red = 2;
    elseif r < 128
        red = 3;
    elseif r < 160
        red = 4;
    elseif r < 192
        red = 5;
    elseif r < 224
        red = 7;
    else
        red = 7;
    end
    
    % GREEN 3 bits
    if g < 32
        green = 1;
    elseif g < 64
        green = 2;
    elseif g < 96
        green = 2;
    elseif g < 128
        green = 3;
    elseif g < 160
        green = 4;
    elseif g < 192
        green = 5;
    elseif g < 224
        green = 7;
    else
        green = 7;
    end
    
    % BLUE 2 bits
    if b < 64
        blue = 0;
    elseif b < 128
        blue = 1;
    elseif b < 192
        blue = 2;
    else
        blue = 3;
    end
    
    color8 = red * 32 + green * 4 + blue;  % 8-bit color
    if color8 == 0x24
        color8= 0x00;
    end
    
    fprintf(fid, '0x%02X', color8);
    if y < width
        fprintf(fid, ', ');
    end

    end
if x < height
    fprintf(fid, '},\n');
else
    fprintf(fid, '}\n');
end
end
fprintf(fid, '};\n');
fclose(fid);
disp('Done! Please check');
disp(output_file);

end